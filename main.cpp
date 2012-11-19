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
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Use nearest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Use nearest mimap
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

    Particle();
    void Update();
};

Particle::Particle() {
    // Set the position
    destx = desty = 0;
    x = rand()%displayWidth;
    y = rand()%displayHeight;

    // Set the speed within the given range
    float maxStartSpeed = fabs(Config.GetFloat("particle:maxStartSpeed",5.0));
    float fractionOfRandSpace = (maxStartSpeed*2)/RAND_MAX;
    xspeed = rand()*fractionOfRandSpace-maxStartSpeed;
    yspeed = rand()*fractionOfRandSpace-maxStartSpeed;

    // Set the color
    colorAnimOffset = (rand()%360)*DTR;
    red = green = blue = 0;
}

void Particle::Update()
{

    if (spaceDown)
    {
        xspeed += (destx-x)/320.0f;
        yspeed += (desty-y)/240.0f;
        xspeed *= .98f;
        yspeed *= .98f;
    }
    else
    {
        xspeed += ((rand()%101)/400.0f)-.125f;
        yspeed += ((rand()%101)/400.0f)-.125f;
    }

    x += xspeed;
    y += yspeed;
    if (x < 0)
    {
        x = 0;
        xspeed = -xspeed;
    }
    if (x > displayWidth)
    {
        x = displayWidth;
        xspeed = -xspeed;
    }
    if (y < 0)
    {
        y = 0;
        yspeed = -yspeed;
    }
    if (y > displayHeight)
    {
        y = displayHeight;
        yspeed = -yspeed;
    }
}









class ParticleMap {
    private:
    float colorAnim;
    unsigned int numParticles;
    Particle* theParticles;
    GLuint particleTex;
    GLuint vbo;
    public:
    ParticleMap();
    ~ParticleMap();
    void StepEvent();
    void DrawEvent();
};

ParticleMap::ParticleMap()
{
    particleTex = LoadAlphaMap(Config.GetString("particle:texture","particle.png").c_str());
    numParticles = Config.GetInt("map:numParticles",500);

    colorAnim = 0;

    int width, height;
    unsigned char *data;
    corona::Image* image = corona::OpenImage(Config.GetString("map:image","theMap.glow").c_str(), corona::PF_R8G8B8);
    unsigned char error[3] = { 0, 0, 0 };
    if (!image) {
        width  = 1;
        height = 1;
        data = error;
    }
    else
    {
        corona::FlipImage(image, 45);

        width  = image->getWidth();
        height = image->getHeight();
        data = (unsigned char*)image->getPixels();
    }

    unsigned int blackPixels = 0;
    for (int i = 0; i < width*height; i += 1)
    {
        if (data[i*3] == 0) blackPixels += 1;
    }

    float pixelSkip = (float)blackPixels / numParticles;

    theParticles = new Particle[numParticles];
    unsigned int particleCount = 0;
    float blackPixelCount = 0;

    for (int i = 0; i < width*height*3; i += 3)
    {
        if (data[i] == 0)
        {
            if (blackPixelCount >= pixelSkip)
            {
                theParticles[particleCount].destx = (i/3)%width;
                theParticles[particleCount].desty = (i/3)/width;
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
        if (theParticles[i].destx == 0 && theParticles[i].desty == 0)
        {
            randParticle = rand()%numParticles;
            theParticles[i].destx = theParticles[randParticle].destx;
            theParticles[i].desty = theParticles[randParticle].desty;
        }
    }


    // Initialize the vertex buffer object
/*    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    const GLsizeiptr vertex_size = numParticles*4*2*sizeof(GLfloat);
    const GLsizeiptr color_size = numParticles*3*sizeof(GLubyte);

    glBufferData(GL_ARRAY_BUFFER, vertex_size+color_size, NULL, GL_DYNAMIC_DRAW);*/
}

ParticleMap::~ParticleMap() {
    delete[] theParticles;
    glDeleteTextures(1,&particleTex);
}

void ParticleMap::StepEvent() {
    for (unsigned int i = 0; i < numParticles; i++) {
        theParticles[i].Update();
    }
}

void ParticleMap::DrawEvent()
{
    colorAnim += .01f;
    if (colorAnim > 2*PI) colorAnim -= 2*PI;

    float red = 1-sin(colorAnim);// >= 0 ? sin(colorAnim) : -sin(colorAnim);
    float green = 1-sin(colorAnim+1.04719755f);// >= 0 ? sin(colorAnim+1.04719755f) : -sin(colorAnim+1.04719755f);
    float blue = 1-sin(colorAnim+2.0943951f);// >= 0 ? sin(colorAnim+2.0943951f) : -sin(colorAnim+2.0943951f);
    glBindTexture(GL_TEXTURE_2D, particleTex);
    glBegin(GL_QUADS);
    for (unsigned int i = 0; i < numParticles; i++)
    {
        theParticles[i].red = fabs(sin(colorAnim+theParticles[i].colorAnimOffset));
        theParticles[i].green = fabs(sin(colorAnim+theParticles[i].colorAnimOffset+1.04719755f));
        theParticles[i].blue = fabs(sin(colorAnim+theParticles[i].colorAnimOffset+2.0943951f));
        glColor3f(theParticles[i].red, theParticles[i].green, theParticles[i].blue);
        glTexCoord2f(0.0f,0.0f); glVertex2f(theParticles[i].x-32,theParticles[i].y-32);
        glTexCoord2f(1.0f,0.0f); glVertex2f(theParticles[i].x+32,theParticles[i].y-32);
        glTexCoord2f(1.0f,1.0f); glVertex2f(theParticles[i].x+32,theParticles[i].y+32);
        glTexCoord2f(0.0f,1.0f); glVertex2f(theParticles[i].x-32,theParticles[i].y+32);
    }
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);

    red = sin(colorAnim) >= 0 ? sin(colorAnim) : -sin(colorAnim);
    green = sin(colorAnim+1.04719755f) >= 0 ? sin(colorAnim+1.04719755f) : -sin(colorAnim+1.04719755f);
    blue = sin(colorAnim+2.0943951f) >= 0 ? sin(colorAnim+2.0943951f) : -sin(colorAnim+2.0943951f);

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


            map.StepEvent();







            // ========
            //  Render
            // ========

            glClear(GL_COLOR_BUFFER_BIT);

            glPushMatrix();

            map.DrawEvent();

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

}

// Disable OpenGL
void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent (NULL, NULL);
    wglDeleteContext (hRC);
    ReleaseDC (hWnd, hDC);
}
