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

// Other
bool spaceDown = false;
bool escapeDown = false;

#include "glow_stuff.h"


// Functions and classes
LRESULT CALLBACK WndProc (HWND hWnd, UINT message,
WPARAM wParam, LPARAM lParam);
void EnableOpenGL (HWND hWnd, HDC *hDC, HGLRC *hRC);
void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC);


// System
const float PI = 3.141592654f;
const float dtr = PI/180;







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
