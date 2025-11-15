// D3D9SimpleTeapot.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <stdio.h>
#include "D3D9SimpleTeapot.h"

#define USE_WIN_ANSI
#include "vsconsole.h"

#ifndef D3DRGBA
#define D3DRGBA(r, g, b, a) \
    (   (((long)((a) * 255)) << 24) | (((long)((r) * 255)) << 16) \
    |   (((long)((g) * 255)) << 8) | (long)((b) * 255) \
    )
#endif 

#define MAX_LOADSTRING 100
#define TIMER_ID 1
#define TIMER_MS (1000 / 60)
#define MESH
#define DTM 0
#define DO_PIXO9 0

#if DO_PIXO9
extern "C" __declspec(dllimport) IDirect3D9 * WINAPI PixoDirect3DCreate9(UINT SDKVersion);

inline IDirect3D9 * WINAPI Direct3DCreate9(UINT SDKVersion)
{
    return PixoDirect3DCreate9(SDKVersion);
}
#endif

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

IDirect3DDevice9			*g_pDevice = NULL;
BOOL						g_bDeviceActive = FALSE;
#ifdef MESH
ID3DXMesh					*g_pTeapot = NULL;
#endif
IDirect3DVertexBuffer9		*g_pVertexBuffer = NULL;
IDirect3DVertexDeclaration9 *g_pVertexDecl = NULL;
IDirect3DIndexBuffer9		*g_pIndexBuffer = NULL;
IDirect3DVertexShader9		*g_pVShader = NULL;
IDirect3DPixelShader9		*g_pPShader = NULL;
ID3DXEffect*				g_pEffect9 = NULL;  
float						g_fRot = 0.0f;
IDirect3DTexture9			*g_pDiffuseTex = NULL;
IDirect3DTexture9*			g_pNormalTex = NULL;
UINT						g_wire = 0;   //default is solid, wire is false
int							g_result = 0; //0: PASS, 1: FAIL
HWND g_hWnd; //R

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL                InitDevice(HWND hWnd);
void                Render();
void                CloseDevice();

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	LPWSTR *sArglist; //list of arguments expected: 1 "notex", "tex" or "texnorm"
	int nArgs;
   
	sArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if( NULL == sArglist )
	{
		return 0; //error
	}

 	// TODO: Place code here.

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_D3D9EMPTYWINDOW, szWindowClass, MAX_LOADSTRING);

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
		return (g_result = 1);
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_D3D9EMPTYWINDOW));

	// Force Init
	WndProc(g_hWnd, WM_COMMAND, 98307, NULL);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

    CloseDevice();

	//return (int) msg.wParam;
	return g_result;
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
	wcex.hIcon			= (HICON)LoadImage(hInstance, 
									MAKEINTRESOURCE(IDI_ICON1),
									IMAGE_ICON,
									0, //GetSystemMetrics(SM_CXSMICON),
									0, //GetSystemMetrics(SM_CYSMICON),
									LR_DEFAULTCOLOR);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_D3D9EMPTYWINDOW);
	wcex.lpszClassName	= szWindowClass;
//	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hIconSm		= (HICON)LoadImage(hInstance, 
									MAKEINTRESOURCE(IDI_ICON1),
									IMAGE_ICON,
									0, //GetSystemMetrics(SM_CXSMICON),
									0, //GetSystemMetrics(SM_CYSMICON),
									LR_DEFAULTCOLOR);

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
      CW_USEDEFAULT, 0, 320, 320, NULL, NULL, hInstance, NULL);
   
   // Set the global hWnd so that the app can be init-ed
   g_hWnd = hWnd;

   if (!hWnd)
   {
      g_result = 1; //test status FAIL
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
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
        case ID_FILE_INITD3D:
			if (g_pDevice)
			{
				MessageBox(hWnd, TEXT("D3D9 Device already initialized"), 
				TEXT("Init Skipped"), MB_OK | MB_ICONWARNING);
			}
			else
			{
				g_bDeviceActive = InitDevice(hWnd);
				if (FALSE == g_bDeviceActive)
				{
					g_result = 1; //FAIL
					MessageBox(hWnd, TEXT("Failed to Initialize D3D9"), 
                           TEXT("Init Failure"), MB_OK | MB_ICONERROR);
					sprintf((char*)pText, "HR %d\n",g_bDeviceActive);
					OutputDebugString (pText);
				}
			}
            break;

        case ID_FILE_CLOSED3D:
            KillTimer(hWnd, TIMER_ID);
            CloseDevice();
            g_bDeviceActive = FALSE;
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

            if (g_bDeviceActive)
            {
                if (width == 0 || height == 0)
                {
                    // Minimized (or zero size)
                    CloseDevice();
                }
                else if (g_pDevice == 0)
                {
                    // Newly un-minimized
                    g_bDeviceActive = InitDevice(hWnd);
                }
                else
                {
                    CloseDevice();
                    g_bDeviceActive = InitDevice(hWnd);
                }
            }
        }
        break;

    case WM_TIMER:
        if (wParam == TIMER_ID)
        {
            // Force a repaint
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;

	case WM_PAINT:
        if (g_pDevice)
        {
            ValidateRect(hWnd, NULL);
#if DTM
			if (g_fRot < 10.0)
			{
				Render();
				// Blt to the screen
				g_pDevice->Present( NULL, NULL, 0, NULL);
			}
			else
			{
				KillTimer(hWnd, TIMER_ID);
				CloseDevice();
				PostQuitMessage(g_result);
			}
#else
			Render();
			// Blt to the screen
			g_pDevice->Present( NULL, NULL, 0, NULL);

#endif
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

    case WM_ERASEBKGND:
        if (g_pDevice)
        {
            break;
        }
        // Fall through to default

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

HRESULT CreateTeapot()
{
   HRESULT hr = S_OK;

#ifdef MESH

   struct TeapotVertex
    {
        D3DXVECTOR3 pos;
        D3DXVECTOR3 n;
        D3DXVECTOR2 uv;
        D3DXVECTOR3 tangent;
        D3DXVECTOR3 binormal;
    };
    TeapotVertex *pData;

    ID3DXMesh *pTeapot;
    if (FAILED( hr = D3DXCreateTeapot(g_pDevice, &pTeapot, NULL) ) )
		return hr;

    if (FAILED( hr = pTeapot->CloneMeshFVF(D3DXMESH_MANAGED | D3DXMESH_32BIT, //remove dynamic is not needed
                                    D3DFVF_XYZ | D3DFVF_NORMAL |	//Postion and Normal 
                                    D3DFVF_TEX3 |					//3 sets of texture co-ordinates
                                    D3DFVF_TEXCOORDSIZE2(0) |		//UV tex coords
                                    D3DFVF_TEXCOORDSIZE3(1) |		//tangent
                                    D3DFVF_TEXCOORDSIZE3(2),		//binormal
                                    g_pDevice, &g_pTeapot) ) )
		return hr;

    pTeapot->Release();
	pTeapot = NULL;

    if (FAILED (hr = g_pTeapot->LockVertexBuffer(0, (void**) &pData) ) )
		return hr; //if failed, teapot will not be as expected

    UINT numVerts = g_pTeapot->GetNumVertices();

    D3DXVECTOR3 vNorm;
    float theta, phi, sintheta;

    // compute the spherical-tent texture coordinates as well
    // as the tangent coordinate frame
    for (UINT i=0; i < numVerts; i++)
    {
        // normalize the teapot vertex
        D3DXVec3Normalize( &vNorm, &pData[i].pos );
        
        theta = acosf(vNorm.y);
        phi = atan2f(vNorm.x, vNorm.z);

        sintheta = sqrtf(1.F-vNorm.y*vNorm.y);

        pData[i].uv.x = theta / D3DX_PI;
        pData[i].uv.y = phi / 2.F / D3DX_PI + 0.5F;

        pData[i].uv.x = (pData[i].uv.x > 0.5)?(1.F-pData[i].uv.x):(pData[i].uv.x);
        pData[i].uv.y = (pData[i].uv.y > 0.5)?(1.F-pData[i].uv.y):(pData[i].uv.y);

        pData[i].uv.x *= 3.F;
        pData[i].uv.y *= 3.F;
        
        pData[i].tangent.x = -sinf(phi)*sintheta;
        pData[i].tangent.y =  cosf(phi)*sintheta;
        pData[i].tangent.z =  0.F;

        D3DXVec3Cross(&pData[i].binormal, &pData[i].n, &pData[i].tangent);
        D3DXVec3Cross(&pData[i].tangent, &pData[i].binormal, &pData[i].n);

        D3DXVec3Normalize(&pData[i].binormal, &pData[i].binormal);
        D3DXVec3Normalize(&pData[i].tangent, &pData[i].tangent);
        D3DXVec3Normalize(&pData[i].n, &pData[i].n);
    }

    hr = g_pTeapot->UnlockVertexBuffer();

#else

#endif

    return hr; 
}

BOOL InitDevice(HWND hWnd)
{
    BOOL result = TRUE;
    HRESULT hr;
    RECT rc;
    GetClientRect( hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    UINT createDeviceFlags = 0;
    IDirect3D9 *pD3D9 = NULL;
    D3DPRESENT_PARAMETERS d3dpp;
	D3DVIEWPORT9 vp;
	//D3DVERTEXELEMENT9 decls[sizeof(float)*6];

	//Create the D3D9 Object
	pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (NULL == pD3D9)
    {
        result = FALSE;
        goto exit;
    }

	//Set the Present Parameters
    memset(&d3dpp, 0, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;

	//Create the D3D9 Device
    pD3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, 
                        D3DCREATE_HARDWARE_VERTEXPROCESSING,
                        &d3dpp, &g_pDevice);
    if (NULL == g_pDevice)
    {
        result = FALSE;
        goto exit;
    }

	//Create the Teapot Object
	if (FAILED( hr = CreateTeapot() ) )
	{
		result = FALSE;
		goto exit;
	}

	//Load the effect file and the texture maps
    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;
	LPD3DXBUFFER pErrors;
    if (FAILED (hr = D3DXCreateEffectFromFile( g_pDevice, L".\\D3D9TeapotTech.fx", NULL, NULL, dwShaderFlags, 
                                        NULL, &g_pEffect9, &pErrors ) ) )
	{
		result = FALSE; //failure to load effect => crashing app, so quit
		OutputDebugStringA("Error!\n");
		OutputDebugStringA((char *)pErrors->GetBufferPointer());
		goto exit;
	}

	if (g_wire)
		if (FAILED (hr = g_pDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME ) ) )
		{
			result = FALSE;
			goto exit;
		}

	if (FAILED (hr = D3DXCreateTextureFromFile(g_pDevice, L".\\LRB_Tile.dds", &g_pDiffuseTex)) )
	{
		result = FALSE; //failure to load texture => crashing app, so quit
		goto exit;
	}
	if (FAILED (hr = D3DXCreateTextureFromFile(g_pDevice, L".\\LRB_Tile_Normal.dds", &g_pNormalTex)) )
	{
		result = FALSE; //failure to load texture => crashing app, so quit
		goto exit;
	}

    // Set the viewport
	RECT rect;
    GetClientRect(hWnd, &rect);
    vp.X = 0;
    vp.Y = 0;
    vp.Width = rect.right - rect.left;
    vp.Height = rect.bottom - rect.top;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    g_pDevice->SetViewport(&vp);

    // Setup timer to refresh window
    SetTimer(hWnd, TIMER_ID, TIMER_MS, NULL);

exit:
    if (FALSE == result)
    {
        CloseDevice();
    }

    return (result);
}



void Render()
{
		HRESULT hr = true;

		D3DXMATRIXA16 mWorld;
		D3DXMATRIXA16 mView;
		D3DXMATRIXA16 mProj;
		D3DXMATRIXA16 mWorldView;
		D3DXMATRIXA16 mWorldViewProjection;

        D3DXMatrixIdentity( &mWorld );
		
		g_fRot += 0.01f; // fixed for now

		D3DXMatrixRotationYawPitchRoll( &mWorld, g_fRot, g_fRot, g_fRot );
		//g_pDevice->SetTransform( D3DTS_WORLD, &mWorld );
		
		// Setup the view parameters
		D3DXVECTOR3 vecEye(6.0f, 0.0f, 6.0f);
		D3DXVECTOR3 vecAt (0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 vecUp (0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH( &mView, &vecEye, &vecAt, &vecUp );
		//g_pDevice->SetTransform( D3DTS_VIEW, &mView );

		D3DXMatrixPerspectiveFovLH(&mProj, (40.0f/180.0f)*D3DX_PI, 1.0f, 0.1f, 10.0f);
		//g_pDevice->SetTransform( D3DTS_PROJECTION, &mProj);

		//Set the effect Matrices
		if (FAILED( hr = g_pEffect9->SetMatrix( "g_mWorld", &mWorld ) ) )
			g_result = 1;

		if (FAILED( hr = g_pEffect9->SetMatrix( "g_mView", &mView ) ) ) 
			g_result = 1;

		mWorldView = mWorld*mView;
		if (FAILED( g_pEffect9->SetMatrix( "g_mWorldView", &mWorldView ) ) )
			g_result = 1;

		mWorldViewProjection = mWorldView*mProj;
		if (FAILED( g_pEffect9->SetMatrix( "g_mWorldViewProjection", &mWorldViewProjection ) ) )
			g_result = 1;

		if (FAILED( g_pEffect9->SetVector( "g_vEyePos", &(D3DXVECTOR4(vecEye, 1))) ) )
			g_result = 1;

		// Clear the back buffer
		D3DCOLOR color = D3DRGBA(0.5f, 0.25f, 0.5f, 1.0f);
        if (FAILED( hr = g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER , color, 1.0f, 0) ) )
			g_result = 1;

		//Load/Set appropriate textures and techniques
		if (FAILED( hr = g_pEffect9->SetTechnique( "RenderScene_GeometrynLightsOnly" ) ) ) 
			g_result = 1;

		if( SUCCEEDED( g_pDevice->BeginScene() ) )
		{
			UINT passes;
			if (FAILED( hr = g_pEffect9->Begin(&passes, 0) ) ) 
				g_result = 1;
			for (UINT i=0; i < passes; i++)
			{
				if (FAILED( hr = g_pEffect9->BeginPass(i) ) ) 
					g_result = 1;
				if (FAILED( hr = g_pTeapot->DrawSubset(0) ) ) 
					g_result = 1;
				if (FAILED( hr = g_pEffect9->EndPass() ) ) 
					g_result = 1;
			}
	        
			if (FAILED( hr = g_pEffect9->End() ) ) 
				g_result = 1;
			
			if (FAILED( hr = g_pDevice->EndScene() ) ) 
				g_result = 1;
		}
		else 
			g_result = 1;
}

void CloseDevice()
{
	if( g_pVertexBuffer )
    {
        g_pVertexBuffer->Release();
        g_pVertexBuffer = 0;
    }

	if( g_pTeapot )
	{
        g_pTeapot->Release();
        g_pTeapot = 0;
    }

    if( g_pPShader )
    {
        g_pPShader->Release();
        g_pPShader = 0;
    }

    if( g_pVShader )
    {
        g_pVShader->Release();
        g_pVShader = 0;
    }

	if( g_pEffect9 )
    {
        g_pEffect9->Release();
        g_pEffect9 = 0;
    }

	if( g_pDiffuseTex )
    {
        g_pDiffuseTex->Release();
        g_pDiffuseTex = 0;
    }

    if( g_pVertexDecl )
    {
        g_pVertexDecl->Release();
        g_pVertexDecl = 0;
    }

    if( g_pDevice )
    {
        __try
        {
            __try
            {
                g_pDevice->Release();
            }
            __finally
            {
                g_pDevice = 0;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            OutputDebugString(TEXT("Error: Crashed on g_pDevice->Release()\n"));
        }
    }
}

