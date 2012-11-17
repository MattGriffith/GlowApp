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



// Functions and classes
LRESULT CALLBACK WndProc (HWND hWnd, UINT message,
WPARAM wParam, LPARAM lParam);
void EnableOpenGL (HWND hWnd, HDC *hDC, HGLRC *hRC);
void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC);


// System
const float PI = 3.141592654f;
const float dtr = PI/180;


// Other
bool spaceDown = false;
bool escapeDown = false;



float nmGetAngle(float x1, float y1, float x2, float y2)
{
    if (x2 == x1)
    {
        if ( y1 > y2 ) return 270;
        if ( y1 < y2 ) return 90;
    }
    if (y1 == y2)
    {
        if ( x2 < x1 ) return 180;
        if ( x2 > x1 ) return 0;
    }

    float angle = atan2(y2-y1, x2-x1);
    angle *= dtr;

    if ( angle < 0 ) {
        angle += 360;
    }

    return angle;
}













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



struct Particle {
    float x, y, xspeed, yspeed, red, green, blue, colorAnim;
    int destx, desty;

    Particle();
    void Update(bool goToDest);
};

Particle::Particle() {
    destx = desty = 0;
    x = rand()%641;
    y = rand()%481;
    xspeed = rand()%10-5;
    yspeed = rand()%10-5;
    colorAnim = (rand()%360)*dtr;
    red = sin(colorAnim) >= 0 ? sin(colorAnim) : -sin(colorAnim);
    green = sin(colorAnim+1.04719755f) >= 0 ? sin(colorAnim+1.04719755f) : -sin(colorAnim+1.04719755f);
    blue = sin(colorAnim+2.0943951f) >= 0 ? sin(colorAnim+2.0943951f) : -sin(colorAnim+2.0943951f);
}

void Particle::Update(bool goToDest)
{
    colorAnim += .01f;
    red = sin(colorAnim) >= 0 ? sin(colorAnim) : -sin(colorAnim);
    green = sin(colorAnim+1.04719755f) >= 0 ? sin(colorAnim+1.04719755f) : -sin(colorAnim+1.04719755f);
    blue = sin(colorAnim+2.0943951f) >= 0 ? sin(colorAnim+2.0943951f) : -sin(colorAnim+2.0943951f);

    if (goToDest)
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
    if (x > 640)
    {
        x = 640;
        xspeed = -xspeed;
    }
    if (y < 0)
    {
        y = 0;
        yspeed = -yspeed;
    }
    if (y > 448)
    {
        y = 448;
        yspeed = -yspeed;
    }
}


class ParticleMap {
    private:
    float colorAnim;
    unsigned int particleNum;
    Particle* theParticles;
    GLuint particleTex;
    public:
    ParticleMap();
    ~ParticleMap();
    void StepEvent();
    void DrawEvent();
};

ParticleMap::ParticleMap()
{
    particleTex = LoadAlphaMap("particle.png");


    particleNum = 500;


    colorAnim = 0;

    int width, height;
    unsigned char *data;
    corona::Image* image = corona::OpenImage("theMap.glow", corona::PF_R8G8B8);
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

    float pixelSkip = (float)blackPixels / particleNum;

    theParticles = new Particle[particleNum];
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
    for (unsigned int i = 0; i < particleNum; i++)
    {
        if (theParticles[i].destx == 0 && theParticles[i].desty == 0)
        {
            randParticle = rand()%particleNum;
            theParticles[i].destx = theParticles[randParticle].destx;
            theParticles[i].desty = theParticles[randParticle].desty;
        }
    }
}

ParticleMap::~ParticleMap() {
    delete[] theParticles;
    glDeleteTextures(1,&particleTex);
}

void ParticleMap::StepEvent()
{
    bool GoToDest = spaceDown;
    for (unsigned int i = 0; i < particleNum; i++)
    {
        theParticles[i].Update(GoToDest);
    }
}

void ParticleMap::DrawEvent()
{
    colorAnim += .01f;
    float red = 1-sin(colorAnim);// >= 0 ? sin(colorAnim) : -sin(colorAnim);
    float green = 1-sin(colorAnim+1.04719755f);// >= 0 ? sin(colorAnim+1.04719755f) : -sin(colorAnim+1.04719755f);
    float blue = 1-sin(colorAnim+2.0943951f);// >= 0 ? sin(colorAnim+2.0943951f) : -sin(colorAnim+2.0943951f);
    glBindTexture(GL_TEXTURE_2D, particleTex);
    glBegin(GL_QUADS);
    for (unsigned int i = 0; i < particleNum; i++)
    {
        glColor3f(theParticles[i].red, theParticles[i].green, theParticles[i].blue);
        glTexCoord2f(0.0f,0.0f); glVertex2f(theParticles[i].x-32,theParticles[i].y-32);
        glTexCoord2f(1.0f,0.0f); glVertex2f(theParticles[i].x+32,theParticles[i].y-32);
        glTexCoord2f(1.0f,1.0f); glVertex2f(theParticles[i].x+32,theParticles[i].y+32);
        glTexCoord2f(0.0f,1.0f); glVertex2f(theParticles[i].x-32,theParticles[i].y+32);
    }
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}




// =========
//  WinMain
// =========


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int iCmdShow)
{
    WNDCLASS wc;
    HWND hWnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;

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

    // Get the screen resolution
    RECT desktop;
    RECT screen;
    GetWindowRect(GetDesktopWindow(),&desktop);
    screen.left = (desktop.right+desktop.left)/2-320;
    screen.top = (desktop.bottom+desktop.top)/2-240;
    screen.right = 640;
    screen.bottom = 480;

    // Create window
    hWnd = CreateWindowEx (0, "TroidGlow", "Troid Glow by Troid92",
    WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
        /*WS_SYSMENU | WS_POPUP,*/ screen.left, screen.top, screen.right, screen.bottom,
        NULL, NULL, hInstance, NULL);

    if (hWnd == 0) {
        MessageBox(NULL, "Error creating a window.", "Error", MB_OK | MB_ICONWARNING);
        return 0;
    }


    srand(timeGetTime());


    // Enable OpenGL
    EnableOpenGL (hWnd, &hDC, &hRC);




    glEnable( GL_TEXTURE_2D );
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glOrtho(0,640,0,480,-1,1);
    glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0,0,640,480);











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
