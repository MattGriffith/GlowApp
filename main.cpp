/*

   ============
   TROID RACING
    By Troid92
   ============

*/

#include <windows.h>
#include <math.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <fstream>
#include <corona.h>
#include <gl/glext.h>

#include "config.h"

#define MIN_WIDTH 64
#define MIN_HEIGHT 64

// Universal constants
static const float PI = 3.141592654f;
static const float DTR = PI/180; // Degrees To Radians

// System settings
static ConfigFile Config;
static int displayWidth = 640;
static int displayHeight = 480;

// VBO extension
PFNGLGENBUFFERSARBPROC glGenBuffers = NULL;
PFNGLBINDBUFFERARBPROC glBindBuffer = NULL;
PFNGLBUFFERDATAARBPROC glBufferData = NULL;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffers = NULL;

GLuint LoadAlphaMap(const char* filename) {
    // Load the image
    corona::Image* image = corona::OpenImage(filename, corona::PF_R8G8B8A8);
    if (image == NULL) return 0;
    corona::FlipImage(image, 45);

    int width  = image->getWidth();
    int height = image->getHeight();
    int numBytes = width*height*4;
    unsigned char *data = (unsigned char*)image->getPixels();

    // Turn it into an alpha map
    for (int byte = 0; byte < numBytes; byte += 4) {
        data[byte+3] = data[byte]; // Alpha value comes from the image's red value
        data[byte] = data[byte+1] = data[byte+2] = 255; // Every pixel is white
    }

    // Create a new OpenGL texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Give it some basic properties
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // Mix colors and texture
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST); // Use nearest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST); // Use nearest mimap
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrap texture over edge
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrap texture over edge

    // Create the mipmaps for this texture
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    delete data;
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}








static bool spaceDown = false;
static bool escapeDown = false;





struct Particle {
    float x, y, xspeed, yspeed, red, green, blue, colorAnimOffset;
    int destx, desty;
};


class ParticleMap {
    public:

    ParticleMap();
    ~ParticleMap();
    void Update();
    void Render();

    private:

    float colorAnim;
    unsigned int numParticles;
    int particleHalfWidth, particleHalfHeight;
    bool manyColors;

    Particle* particleList;
    GLfloat* vertexList;
    GLfloat* colorList;
    GLuint texCoordVBO;
    GLuint particleTex;

    inline void CopyToGLArrays(unsigned int index);
};


inline void ParticleMap::CopyToGLArrays(unsigned int index) {
    Particle& p = particleList[index];

    // Fill in the colors
    if (manyColors) {
        unsigned int colorIt = index*12;
        colorList[colorIt] = colorList[colorIt+3] = colorList[colorIt+6] = colorList[colorIt+9] = p.red;
        colorList[colorIt+1] = colorList[colorIt+4] = colorList[colorIt+7] = colorList[colorIt+10] = p.green;
        colorList[colorIt+2] = colorList[colorIt+5] = colorList[colorIt+8] = colorList[colorIt+11] = p.blue;
    }

    // Fill in the vertices
    GLfloat top = p.y+particleHalfHeight;
    GLfloat bottom = p.y-particleHalfHeight;
    GLfloat left = p.x-particleHalfWidth;
    GLfloat right = p.x+particleHalfWidth;
    unsigned int vertexIt = index*8;
    vertexList[vertexIt] = left; vertexList[vertexIt+1] = bottom;
    vertexList[vertexIt+2] = right; vertexList[vertexIt+3] = bottom;
    vertexList[vertexIt+4] = right; vertexList[vertexIt+5] = top;
    vertexList[vertexIt+6] = left; vertexList[vertexIt+7] = top;
}

ParticleMap::ParticleMap() {
    // Load information from the config file, including the texture
    particleTex = LoadAlphaMap(Config.GetString("particle:texture","particle.png").c_str());
    numParticles = Config.GetInt("map:numParticles",500);
    if (numParticles < 0) numParticles = 0;
    particleHalfWidth = Config.GetInt("particle:width",64)/2;
    particleHalfHeight = Config.GetInt("particle:height",64)/2;
    manyColors = !Config.GetBool("particle:uniformColor",true);
    colorAnim = 0;

    // Allocate memory for the particles, and the GL arrays
    particleList = new Particle[numParticles];
    vertexList = new GLfloat[numParticles*4*2];
    if (manyColors) colorList = new GLfloat[numParticles*4*3];
    GLfloat* texCoordList = new GLfloat[numParticles*4*2];
    if (particleList == NULL || vertexList == NULL || texCoordList == NULL) exit(1);
    if (manyColors && colorList == NULL) exit(1);

    // Initialize the particles and GL arrays
    float maxStartSpeed = fabs(Config.GetFloat("particle:maxStartSpeed",5.0));
    float fractionOfRandSpace = (maxStartSpeed*2)/RAND_MAX;
    for (unsigned int particleIt = 0; particleIt < numParticles; particleIt++) {
        Particle& p = particleList[particleIt];

        // Set the position
        p.destx = p.desty = 0;
        p.x = rand()%displayWidth;
        p.y = rand()%displayHeight;

        // Set the speed within the given range
        p.xspeed = rand()*fractionOfRandSpace-maxStartSpeed;
        p.yspeed = rand()*fractionOfRandSpace-maxStartSpeed;

        // Set the color
        p.colorAnimOffset = (rand()%360)*DTR;
        p.red = p.green = p.blue = 1;

        // Set the GL arrays
        CopyToGLArrays(particleIt);
        unsigned int texCoordIt = particleIt*4*2;
        texCoordList[texCoordIt] = 0;
        texCoordList[texCoordIt+1] = 0;
        texCoordList[texCoordIt+2] = 1;
        texCoordList[texCoordIt+3] = 0;
        texCoordList[texCoordIt+4] = 1;
        texCoordList[texCoordIt+5] = 1;
        texCoordList[texCoordIt+6] = 0;
        texCoordList[texCoordIt+7] = 1;
    }

    // Generate the VBO for the texture coordinates
    glGenBuffers(1,&texCoordVBO);
    glBindBuffer(GL_ARRAY_BUFFER,texCoordVBO);
    glBufferData(GL_ARRAY_BUFFER,numParticles*2*4*sizeof(GLfloat),texCoordList,GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    delete[] texCoordList;

    int width, height;
    unsigned char *data;
    corona::Image* image = corona::OpenImage(Config.GetString("map:image","map.glow").c_str(), corona::PF_R8G8B8);
    unsigned char error[3] = { 0, 0, 0 };
    if (!image) {
        width  = 1;
        height = 1;
        data = error;
    }
    else {
        corona::FlipImage(image, 45);

        width  = image->getWidth();
        height = image->getHeight();
        data = (unsigned char*)image->getPixels();
    }

    unsigned int blackPixels = 0;
    for (int i = 0; i < width*height; i += 1) {
        if (data[i*3] == 0) blackPixels += 1;
    }

    float pixelSkip = (float)blackPixels / numParticles;

    unsigned int particleCount = 0;
    float blackPixelCount = 0;

    // Center the map in the window
    int offsetX = (displayWidth-width)/2;
    int offsetY = (displayHeight-height)/2;

    for (int i = 0; i < width*height*3; i += 3)
    {
        if (data[i] == 0)
        {
            if (blackPixelCount >= pixelSkip)
            {
                particleList[particleCount].destx = (i/3)%width + offsetX;
                particleList[particleCount].desty = (i/3)/width + offsetY;
                particleCount += 1;
                blackPixelCount = 0;
            }
            blackPixelCount += 1;
        }
    }

    delete image;

    unsigned int randParticle;
    for (unsigned int i = 0; i < numParticles; i++)
    {
        if (particleList[i].destx == 0 && particleList[i].desty == 0)
        {
            randParticle = rand()%numParticles;
            particleList[i].destx = particleList[randParticle].destx;
            particleList[i].desty = particleList[randParticle].desty;
        }
    }
}

ParticleMap::~ParticleMap() {
    delete[] particleList;
    delete[] vertexList;
    if (manyColors) delete[] colorList;
    glDeleteBuffers(1,&texCoordVBO);
    glDeleteTextures(1,&particleTex);
}

void ParticleMap::Update() {
    for (unsigned int i = 0; i < numParticles; i++) {
        Particle& p = particleList[i];

        if (spaceDown) {
            p.xspeed += (p.destx-p.x)/320.0f;
            p.yspeed += (p.desty-p.y)/240.0f;
            p.xspeed *= .98f;
            p.yspeed *= .98f;
        }
        else {
            p.xspeed += ((rand()%101)/400.0f)-.125f;
            p.yspeed += ((rand()%101)/400.0f)-.125f;
        }

        p.x += p.xspeed;
        p.y += p.yspeed;
        if (p.x < 0) {
            p.x = 0;
            p.xspeed = -p.xspeed;
        }
        else if (p.x > displayWidth) {
            p.x = displayWidth;
            p.xspeed = -p.xspeed;
        }
        if (p.y < 0) {
            p.y = 0;
            p.yspeed = -p.yspeed;
        }
        else if (p.y > displayHeight) {
            p.y = displayHeight;
            p.yspeed = -p.yspeed;
        }

        if (manyColors) {
            float color = colorAnim+particleList[i].colorAnimOffset;
            p.red = fabs(sin(color));
            p.green = fabs(sin(color+PI*.3333333));
            p.blue = fabs(sin(color+PI*.6666666));
        }

        CopyToGLArrays(i);
    }
}

void ParticleMap::Render()
{
    colorAnim += .01f;
    if (colorAnim > 2*PI) colorAnim -= 2*PI;

    if (!manyColors) {
        float red = 1-sin(colorAnim);// >= 0 ? sin(colorAnim) : -sin(colorAnim);
        float green = 1-sin(colorAnim+1.04719755f);// >= 0 ? sin(colorAnim+1.04719755f) : -sin(colorAnim+1.04719755f);
        float blue = 1-sin(colorAnim+2.0943951f);// >= 0 ? sin(colorAnim+2.0943951f) : -sin(colorAnim+2.0943951f);*/
        glColor3f(red,green,blue);
    }

    if (manyColors) glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, vertexList);
    if (manyColors) glColorPointer(3, GL_FLOAT, 0, colorList);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);

    glBindTexture(GL_TEXTURE_2D, particleTex);
    glDrawArrays(GL_QUADS,0,numParticles*4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (manyColors) glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glColor3f(1,1,1);
}




// =========
//  WinMain
// =========


// Functions and classes
LRESULT CALLBACK WndProc (HWND hWnd, UINT message,
WPARAM wParam, LPARAM lParam);
void EnableOpenGL (HWND hWnd, HDC *hDC, HGLRC *hRC);
void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int iCmdShow)
{
    WNDCLASS wc;
    HWND hWnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;

    Config.Open("glow.ini");

    // Register window
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);//ExtractIcon (NULL, "NetMission.ico", 0);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "TroidGlow";
    RegisterClass(&wc);

    // Get the screen resolution and center the window
    RECT desktop;
    displayWidth = Config.GetInt("display:width",640);
    displayHeight = Config.GetInt("display:height",480);
    if (displayWidth < MIN_WIDTH) displayWidth = MIN_WIDTH;
    if (displayHeight < MIN_HEIGHT) displayHeight = MIN_HEIGHT;
    int windowLeft, windowTop;
    GetWindowRect(GetDesktopWindow(),&desktop);
    windowLeft = (desktop.right+desktop.left-displayWidth)/2;
    windowTop = (desktop.bottom+desktop.top-displayHeight)/2;

    // Create window
    hWnd = CreateWindowEx (0, "TroidGlow",
                            Config.GetString("display:caption","TroidGlow by Matthew Griffith").c_str(),
                            WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
                            windowLeft, windowTop, displayWidth, displayHeight,
                            NULL, NULL, hInstance, NULL);

    if (hWnd == 0) {
        MessageBox(NULL, "Error creating a window.", "Error", MB_OK | MB_ICONWARNING);
        return 0;
    }


    srand(timeGetTime());


    // Enable OpenGL
    EnableOpenGL (hWnd, &hDC, &hRC);




    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    if (Config.GetBool("particle:addBlend",false)) {
        glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    }
    else {
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    }
    glOrtho(0,displayWidth,0,displayHeight,-1,1);

    glClearColor(Config.GetFloat("display:bgRed",0.0f),
                 Config.GetFloat("display:bgGreen",0.0f),
                 Config.GetFloat("display:bgBlue",0.0f),
                 0.0f);

    glViewport(0,0,displayWidth,displayHeight);



    ConfigFile hi;
    hi.Open("glow.ini");







    // Game loop
	glBindTexture(GL_TEXTURE_2D, 0);



	// Objects and stuff go here.
	ParticleMap map;



    bool play = true;
    while (play)
    {
        if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                play = false;
            }
            else
            {
                TranslateMessage (&msg);
                DispatchMessage (&msg);
            }
        }
        else
        {




            if (escapeDown) play = false;


            map.Update();







            // ========
            //  Render
            // ========

            glClear(GL_COLOR_BUFFER_BIT);

            glPushMatrix();

            map.Render();

            glPopMatrix();

            glFlush();

            SwapBuffers (hDC);










            Sleep(16);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL (hWnd, hDC, hRC);



    /* destroy the window explicitly */
    DestroyWindow (hWnd);

    return 0;
}


// =========
//  WndProc
// =========

LRESULT CALLBACK WndProc (HWND hWnd, UINT message,
                          WPARAM wParam, LPARAM lParam)
{

    switch (message) {

        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;

        case WM_KEYDOWN:
            if (wParam == VK_SPACE) spaceDown = true;
            else if (wParam == VK_ESCAPE) escapeDown = true;
            return 0;

        case WM_KEYUP:
            if (wParam == VK_SPACE) spaceDown = false;
            else if (wParam == VK_ESCAPE) escapeDown = false;
            return 0;

        default:
            return DefWindowProc (hWnd, message, wParam, lParam);
    }
}


// =============
//  Definitions
// =============


// Enable OpenGL
void EnableOpenGL (HWND hWnd, HDC *hDC, HGLRC *hRC)
{
    PIXELFORMATDESCRIPTOR pfd;
    int iFormat;

    // get the device context (DC)
    *hDC = GetDC (hWnd);

    // set the pixel format for the DC
    ZeroMemory (&pfd, sizeof (pfd));
    pfd.nSize = sizeof (pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
      PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    iFormat = ChoosePixelFormat (*hDC, &pfd);
    SetPixelFormat (*hDC, iFormat, &pfd);

    // create and enable the render context (RC)
    *hRC = wglCreateContext( *hDC );
    wglMakeCurrent( *hDC, *hRC );

    // Initialize the extensions we're going to use
    glGenBuffers = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
    glBindBuffer = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
    glBufferData = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
    glDeleteBuffers = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");

}

// Disable OpenGL
void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent (NULL, NULL);
    wglDeleteContext (hRC);
    ReleaseDC (hWnd, hDC);
}
