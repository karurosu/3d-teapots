// D3D11SimpleTeapot.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "D3D11SimpleTeapot.h"
#include "GraphicsLayer.h"
#include "SmoothTeapot.h"


#include <stdio.h>

#define MAX_LOADSTRING 100
#define TIMER_ID 1
#define TIMER_MS (1000 / 60)


// Global Variables:
HINSTANCE					hInst;								// current instance
TCHAR						szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR						szWindowClass[MAX_LOADSTRING];			// the main window class name



CSmoothTeapot				*g_Teapot[1]={NULL};	

bool						g_pause=false;
HWND						g_hWnd; //R




// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL                InitDevice(HWND hWnd);
void                Render();
void				ImageCapture(HWND hWnd);//add to capture images


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg={0};
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_D3D11SIMPLETEAPOT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_D3D11SIMPLETEAPOT));

	
// Force Init
	WndProc(g_hWnd, WM_COMMAND, 98307, NULL);
	
	while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
			Render();
			
        }
    }

	
	
	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_D3D11SIMPLETEAPOT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_D3D11SIMPLETEAPOT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass,szTitle,WS_OVERLAPPEDWINDOW,
	   20, 20, 800, 600, NULL, NULL, hInstance, NULL);

	g_hWnd = hWnd;
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_KEYUP:
		switch(wParam)
		{		
			case 67:
				ImageCapture(hWnd);	//using "c"
			break;
			case 80:
				g_pause=!g_pause;	//using "p"
			break;
			case '1':
				g_option = GOURAUD_SHADING;
				Graphics()->SetTechnique(g_option);
			break;
			case '2':
				g_option = PHONG_SHADING;
				Graphics()->SetTechnique(g_option);
			break;
			case '3':
				g_option = TEXTURE;
				Graphics()->SetTechnique(g_option);
			break;
			case '4':
				g_option = UV;
				Graphics()->SetTechnique(g_option);
			break;
			case '5':
				g_option = NORMAL_MAP;
				Graphics()->SetTechnique(g_option);
			break;
			case '6':
				g_option = TOON;
				Graphics()->SetTechnique(g_option);
			break;
			case '7':
				g_option = REFLECT;
				Graphics()->SetTechnique(g_option);
			break;
			case '8':
				g_option = HAIR;
				Graphics()->SetTechnique(g_option);
			break;
		}
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
			case ID_FILE_INITD3D:
				
			if (Graphics())
			{
				MessageBox(hWnd, TEXT("D3D11 Device already initialized"), 
				TEXT("Init Skipped"), MB_OK | MB_ICONWARNING);
			}
			else
			{
				// = InitDevice(hWnd);
				
				if (InitDevice(hWnd) == FALSE)
				{
				
					MessageBox(hWnd, TEXT("Failed to Initialize D3D11"), 
                           TEXT("Init Failure"), MB_OK | MB_ICONERROR);
					//sprintf((char*)pText, "HR %d\n",g_bDeviceActive);
					//outputDebugString (pText);
				}
			}
            break;

        case ID_FILE_CLOSED3D:
            KillTimer(hWnd, TIMER_ID);
            
            //g_bDeviceActive = FALSE;
            InvalidateRect(hWnd, NULL, TRUE);
            break;

		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		
		if (Graphics())
        {
			Render();
		}
		else
		{
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

//---------------------------------------------------------------------------
BOOL InitDevice(HWND hWnd)
{
	RECT rect;
    GetClientRect(hWnd, &rect);
    BOOL result = TRUE;
	CGraphicsLayer::Create(hWnd,rect.right - rect.left,rect.bottom - rect.top);
	if(Graphics()==NULL){return FALSE;}
	
	g_Teapot[0]=new CSmoothTeapot();
	
    return (result);
}


//---------------------------------------------------------------------------
void ImageCapture(HWND hWnd)
{
	HRESULT hr;
	hr=D3DX11SaveTextureToFile(Graphics()->GetDeviceContext(),Graphics()->GetBackBuffer(),D3DX11_IFF_BMP,L".\\CapturedImg.bmp");
}

void Render()
{

	if(!Graphics()){return;}
	Graphics()->ClearBackBuffer();
	Graphics()->ClearStencilBuffer();
	if(!g_pause)g_Teapot[0]->Update(0.001);
	g_Teapot[0]->DrawSmooth();
	Graphics()->Present();	

}
