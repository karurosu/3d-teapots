// D3D10SimpleTeapot.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "D3D10SimpleTeapot.h"
#include "GraphicsLayer.h"
#include "Teapot.h"
#include <stdio.h>


#define MAX_LOADSTRING 100
#define TIMER_ID 1
#define TIMER_MS (1000 / 60)

	

// Global Variables:
HINSTANCE					hInst;									// current instance
TCHAR						szTitle[MAX_LOADSTRING];				// The title bar text
TCHAR						szWindowClass[MAX_LOADSTRING];			// the main window class name
SYSTEMTIME                  g_st;									//system time
int                         g_MilliSecBack;							//system
static const int			NUM_OBJ=3;
CTeapot						*g_Teapot[NUM_OBJ]={NULL};			
//UINT						g_wire = 0;								//default is solid, wire is false
int							g_result = 0;							//0: PASS, 1: FAIL
bool						g_pause=false;
bool						g_BSphere=false;
HWND						g_hWnd; //R
FILE						*g_fOutput = NULL;
int							g_option = 0;


//PRINCIPAL OPTIONS
const int GOURAUD_SHADING = 0;
const int PHONG_SHADING	  = 1;
const int DIFF_TEX		  = 2;
const int REMAP1		  = 3;
const int REMAP2		  = 4;
const int NORMAL_MAP	  = 5;
const int GLOSS_MAP		  = 6;
const int ALPHA_FROM_MAT  = 7;
const int ALPHA_FROM_TEX  = 8;
const int GRASS_MAP		  = 9;

const int REFLECT		  = 10;
const int HAIR			  = 11;





// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL                InitDevice(HWND hWnd);
void                Render();
void				ImageCapture(HWND hWnd);//add to capture images
void				ObjectWorldRT();





int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	char *Str = GetCommandLineA();
	char *Opt = Str;
	while (*Opt && *Opt != ' ') Opt++;
	Opt++;
	g_fOutput = fopen("out.txt", "wt");
	fprintf(g_fOutput, "%s\n", Opt);
	if (Opt)
	{
		if (Opt[0] >= '0' && Opt[0] <= '9')
		{
			g_option = Opt[0] - '0';
		}
		else
		{
			if (strcmp(Opt, "gouraud") == 0)
			{
				g_option = GOURAUD_SHADING;
				fprintf(g_fOutput, "GOURAUD_SHADING\n");
			}
			
			if (strcmp(Opt, "phong") == 0)
			{
				g_option = PHONG_SHADING;
				fprintf(g_fOutput, "PHONG_SHADING\n");
			}

			if (strcmp(Opt, "tex") == 0)
			{
				g_option = DIFF_TEX;
				fprintf(g_fOutput, "DIFF_TEX\n");
			}

			if (strcmp(Opt, "remap1") == 0)
			{
				g_option = REMAP1;
			}

			if (strcmp(Opt, "remap2") == 0)
			{
				g_option = REMAP2;
			}

			if (strcmp(Opt, "texnorm") == 0)
			{
				g_option = NORMAL_MAP;
			}
			if (strcmp(Opt, "gloss") == 0)
			{
				g_option = GLOSS_MAP;
			}
			if (strcmp(Opt, "alphamat") == 0)
			{
				g_option = ALPHA_FROM_MAT;
			}
			if (strcmp(Opt, "alphatx") == 0)
			{
				g_option = ALPHA_FROM_TEX;
			}
			if (strcmp(Opt, "grass") == 0)
			{
				g_option = GRASS_MAP;
			}
		}
	}
 	// TODO: Place code here.
	MSG msg={0};
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_D3D10SIMPLETEAPOT, szWindowClass, MAX_LOADSTRING);
	if (sizeof(void*) == 8)
    {
        wcsncat(szTitle, TEXT("64"), MAX_LOADSTRING);
    }
    else
    {
        wcsncat(szTitle, TEXT("32"), MAX_LOADSTRING);
    }
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_D3D10SIMPLETEAPOT));

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
			Render();//TODO
			//InvalidateRect(g_hWnd, NULL, FALSE);
        }
    }

	
	fclose(g_fOutput);
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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_D3D10SIMPLETEAPOT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_D3D10SIMPLETEAPOT);
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

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   // Set the global hWnd so that the app can be init-ed
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

	TCHAR pText[1024];

	switch (message)
	{
	case WM_KEYUP:
		switch(wParam)
		{
			case '1':
				g_option = GOURAUD_SHADING;
				Graphics()->SetTechnique(0);
			break;
			case '2':
				g_option = PHONG_SHADING;
				Graphics()->SetTechnique(1);
			break;
			case '3':
				g_option = DIFF_TEX;
				Graphics()->SetTechnique(2);
			break;
			case '4':
				g_option = GLOSS_MAP;
				Graphics()->SetTechnique(3);
			break;
			case '5':
				g_option = NORMAL_MAP;
				Graphics()->SetTechnique(4);
			break;
			case '6':
				g_option = ALPHA_FROM_TEX;
				Graphics()->SetTechnique(5);
			break;
			case '7':
				g_option = GRASS_MAP;
				Graphics()->SetTechnique(6);
			break;
			case '8':
				g_option = ALPHA_FROM_MAT;
				Graphics()->SetTechnique(9);
			break;

			case '9':
				g_option = REFLECT;
				Graphics()->SetTechnique(10);
			break;
			case '0':
				g_option = HAIR;
				Graphics()->SetTechnique(11);
			break;
			
			case 67:
				ImageCapture(hWnd);	//using "c"
			break;
			case 80:
				g_pause=!g_pause;	//using "p"
			break;
			case 83:
				//g_BSphere=!g_BSphere;
				//g_pEffect9->SetInt( "g_sphere",g_BSphere);
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
				MessageBox(hWnd, TEXT("D3D9 Device already initialized"), 
				TEXT("Init Skipped"), MB_OK | MB_ICONWARNING);
			}
			else
			{
				// = InitDevice(hWnd);
				
				if (InitDevice(hWnd) == FALSE)
				{
					g_result = 1; //FAIL
					MessageBox(hWnd, TEXT("Failed to Initialize D3D9"), 
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

	case WM_SIZE:
        {
            UINT width = (UINT)LOWORD(lParam);
            UINT height = (UINT)HIWORD(lParam);

        }
        break;

    case WM_TIMER:
        if (wParam == TIMER_ID)
        {
            // Force a repaint
            //InvalidateRect(hWnd, NULL, FALSE);
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
		 KillTimer(hWnd, TIMER_ID);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
//-----------------------------------------------------------------------------
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
	
	int iMapType=0;
	if(g_option==REMAP1){iMapType=1;}
	if(g_option==REMAP2){iMapType=2;}

	g_Teapot[0]=new CTeapot(-1,1.0f,D3DXVECTOR3(0.0,0.0,0.0),iMapType);
	g_Teapot[1]=new CTeapot( 0,2.0f,D3DXVECTOR3(0.0,0.0,2.0),iMapType);
	g_Teapot[2]=new CTeapot( 1,4.0f,D3DXVECTOR3(0.0,0.0,1.0),iMapType);
   
	switch(g_option)
		{
			case GOURAUD_SHADING:
				Graphics()->SetTechnique(0);
			break;
			case PHONG_SHADING:
				Graphics()->SetTechnique(1);
			break;
			case DIFF_TEX:
				Graphics()->SetTechnique(2);
			break;
			case GLOSS_MAP:
				Graphics()->SetTechnique(3);
			break;
			case NORMAL_MAP:
				Graphics()->SetTechnique(4);
			break;
			case ALPHA_FROM_TEX:
				Graphics()->SetTechnique(5);
			break;
			case GRASS_MAP:
				Graphics()->SetTechnique(6);
			break;
			case ALPHA_FROM_MAT:
				Graphics()->SetTechnique(9);
			break;
			case REMAP1:
				Graphics()->SetTechnique(2);
			break;
			case REMAP2:
				Graphics()->SetTechnique(2);
			break;
			case REFLECT:
				Graphics()->SetTechnique(10);
			break;
			case HAIR:
				Graphics()->SetTechnique(11);
			break;
			
		}
    return (result);
}


//---------------------------------------------------------------------------
void ImageCapture(HWND hWnd)
{
	HRESULT hr;
	hr=D3DX10SaveTextureToFile(Graphics()->GetBackBuffer(),D3DX10_IFF_BMP,L".\\CapturedImg.bmp");
}

void Render()
{

	if(!Graphics()){return;}
	Graphics()->ClearBackBuffer();
	Graphics()->ClearStencilBuffer();

	ObjectWorldRT();

	GetSystemTime(&g_st);
	int DeltaTime=g_st.wMilliseconds-g_MilliSecBack;		
	if(DeltaTime<0)
	{
		DeltaTime=1000-g_MilliSecBack+g_st.wMilliseconds;
	}
	g_MilliSecBack=g_st.wMilliseconds;

	for(int i=0;i<NUM_OBJ;i++)
	{
		if(!g_pause)g_Teapot[i]->Update(DeltaTime);
		g_Teapot[i]->Draw();
	}
	
	Graphics()->Present();	

}
//--------------------------------------------------------------------------
void ObjectWorldRT()
{
	D3DXMATRIX R,T,ER;
	D3DXVECTOR3 p;
	//compute the transformation ralated to the parent
	for(int i=0;i<NUM_OBJ;++i)
	{
		p=g_Teapot[i]->m_Tp;
		D3DXMatrixRotationY(&R,g_Teapot[i]->m_Ang);
		D3DXMatrixRotationYawPitchRoll(&ER,g_Teapot[i]->m_ExtAng,g_Teapot[i]->m_ExtAng,g_Teapot[i]->m_ExtAng);
		D3DXMatrixTranslation(&T,p.x,p.y,p.z);
		g_Teapot[i]->m_toParentXFrom=R*T;
	}

	//computing all the transformations to the world 
	for(int i=0;i<NUM_OBJ;++i)
	{
	
		D3DXMatrixIdentity(&g_Teapot[i]->m_toWorldXFrom);
		int k=i;
		while(k!=-1)
		{
			g_Teapot[i]->m_toWorldXFrom*=g_Teapot[k]->m_toParentXFrom;
			k=g_Teapot[k]->m_ParentIdx;
		}
	
		g_Teapot[i]->m_toWorldXFrom=ER*g_Teapot[i]->m_toWorldXFrom;
		
	}
}



