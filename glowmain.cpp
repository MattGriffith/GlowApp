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

#include "NMObjects_1-01.h"
#include "glow_stuff.h"


// Functions and classes
LRESULT CALLBACK WndProc (HWND hWnd, UINT message,
WPARAM wParam, LPARAM lParam);
void EnableOpenGL (HWND hWnd, HDC *hDC, HGLRC *hRC);
void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC);

// Other
bool key[256];
bool keyprev[256];
bool keyn[256];
bool keyp[256];
bool keyr[256];

// System
const float PI = 3.141592654f;
const float dtr = PI/180;
int i;
int j;





/*
class BasicObject : public NMObject
{
    public:
    BasicObject();
    ~BasicObject();
    void StepEvent();
    void DrawEvent();
    void Destroy() { delete this; }
};

BasicObject::BasicObject()
{
    Register("BasicObject",0);
}

void BasicObject::StepEvent()
{
    ;
}

void BasicObject::DrawEvent()
{
    ;
}
*/










class NMFramerate
{
    private:
    unsigned int oneFrame;
    int startTime, endTime, SleepTime;
    unsigned long frame;
    
    public:
    NMFramerate(unsigned int );
    void SetFramerate(unsigned int);
    void StartFrame();
    void EndFrame();
    unsigned long GetFrame();
};

NMFramerate::NMFramerate(unsigned int fps = 30)
{
    SetFramerate(fps);
    startTime = timeGetTime()-oneFrame;
}

void NMFramerate::SetFramerate(unsigned int fps)
{
    if (fps && fps <= 1000)
    {
        oneFrame = 1000/fps;
    }
    else oneFrame = 1000/30;
    frame = 0;
}

void NMFramerate::StartFrame()
{
    if (SleepTime <= 0)
    {
        startTime = timeGetTime();
    }
    else startTime += oneFrame;
}

void NMFramerate::EndFrame()
{
    endTime = timeGetTime();
    SleepTime = oneFrame-(endTime-startTime);
    if (SleepTime > 0 && SleepTime <= oneFrame)
    {
        Sleep(SleepTime);
    }
    frame += 1;
}

unsigned long NMFramerate::GetFrame()
{
    return frame;
}


























// =========
//  WinMain 
// =========


int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine,
                    int iCmdShow)
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
    wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);//ExtractIcon (NULL, "NetMission.ico", 0);
    wc.hCursor = LoadCursor (NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "TroidGlow";
    RegisterClass (&wc);

    // Get the screen resolution
    RECT desktop;
    GetWindowRect(GetDesktopWindow(),&desktop);
    RECT screen;
    screen.left = (desktop.right+desktop.left)/2-320;
    screen.top = (desktop.bottom+desktop.top)/2-240;
    screen.right = 640;
    screen.bottom = 480;

    // Create window
    hWnd = CreateWindowEx (0, "TroidGlow", "Troid Glow by Troid92", 
    WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
        /*WS_SYSMENU | WS_POPUP,*/ screen.left, screen.top, screen.right, screen.bottom,
        NULL, NULL, hInstance, NULL);

    if (hWnd == 0)
    {
        MessageBox(NULL, "Error creating a window.", "Error", MB_OK | MB_ICONWARNING);
        return 0;
    }
    
    
    srand(timeGetTime());
    
    for (i = 0; i < 256; i++)
    {
        key[i] = false;
        keyprev[i] = false;
        keyn[i] = false;
        keyp[i] = false;
        keyr[i] = false;
    }
    
    
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
	new ParticleMap;
    
    NMFramerate FPS = 60;
    
    
    
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
            
            FPS.StartFrame();
            
            
            
            
            // ========
            //  Input: 
            // ========
            
            
            // Keyboard:
            for (i = 0; i < 256; i++)
            {
                // If the key was down
                if (key[i])
                {
                    // ... But came up
                    if (keyn[i])
                    {
                        // Then it's not down
                        key[i] = false;
                        keyn[i] = false;
                    }
                    
                    // If it wasn't down the previous frame, however...
                    // Then it was just pressed
                    if (!keyprev[i]) keyp[i] = true;
                    // Otherwise it wasn't just pressed
                    else keyp[i] = false;
                }
                // If the key wasn't down this frame
                else
                {
                    // Then it wasn't just pressed
                    keyp[i] = false;
                }
                // And keep the key state for the next frame's checking
                keyprev[i] = key[i];
            }
            
            // --------------
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            if (keyp[VK_ESCAPE]) play = false;
            
            
            STEP();
            
            
            
            
            
            
            
            // ========
            //  Render 
            // ========
            
            glClear(GL_COLOR_BUFFER_BIT);
            
            glPushMatrix();
            
            DRAW();
            
            glPopMatrix();
            
            glFlush();
            
            SwapBuffers (hDC);
            
            
            
            
            
            
            
            
            
            
            FPS.EndFrame();
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL (hWnd, hDC, hRC);

    

    /* destroy the window explicitly */
    DestroyWindow (hWnd);

    return msg.wParam;
}


// =========
//  WndProc 
// =========

LRESULT CALLBACK WndProc (HWND hWnd, UINT message,
                          WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
    case WM_CREATE:
        return 0;

    case WM_CLOSE:
        PostQuitMessage (0);
        return 0;

    case WM_DESTROY:
        return 0;

    case WM_KEYDOWN:
        key[wParam] = true;
        return 0;

    case WM_SYSKEYDOWN:
        key[wParam] = true;
        return 0;

    case WM_KEYUP:
        keyn[wParam] = true;
        return 0;

    case WM_SYSKEYUP:
        keyn[wParam] = true;
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
