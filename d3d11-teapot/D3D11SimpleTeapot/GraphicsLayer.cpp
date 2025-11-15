#include "StdAfx.h"
#include "GraphicsLayer.h"

CGraphicsLayer	*CGraphicsLayer::m_pGlobalGLayer=0;

CGraphicsLayer::~CGraphicsLayer(void)
{


}
//-----------------------------------------------------------
CGraphicsLayer *Graphics()
{
	return CGraphicsLayer::GetGraphics();
}
//----------------------------------------------------------------
CGraphicsLayer::CGraphicsLayer(HWND	hwnd)
{
	m_hWnd=hwnd;
	m_pDevice = NULL;
	m_pDeviceContext = NULL;
	
	m_techId = 0;

	//Gouraud
	m_pVSGouraud=NULL;
	m_pPSGouraud=NULL;

	//Smooth shaders
	m_pHSSmooth=NULL;
	m_pDSSmooth=NULL;

	//Default geometry shader
	m_pGSSmooth=NULL;

	m_pPSPhong = NULL;
	m_pPSTexture = NULL;
	m_pPSUV = NULL;
	m_pGSNormal = NULL;
	m_pPSToon = NULL;
	m_pPSReflect = NULL;
    m_pPSHair = NULL;
	m_pGSHair = NULL;
	m_pPSHair = NULL;


	//default is solid, wire is false
	m_wire=0;						
	m_pause=false;
	
	m_pSwapChain = NULL;
	m_pRenderTargetView = NULL;
	m_pGlobalGLayer = this;
	m_pBackBuffer =NULL;
	m_pConstantsBuffer= NULL;
}

void CGraphicsLayer::SetTechnique(UINT id) {
	OutputDebugString(L"Changing technique");
	m_techId = id;
}


void CGraphicsLayer::Create(HWND hWnd,short width,short height)
{
	new CGraphicsLayer(hWnd);
	Graphics()->InitD3D(width,height,32);
}
//---------------------------------------------------------------
void CGraphicsLayer::InitD3D(int Width,int Height,int bpp)
{
	//HRESULT r=0;

	m_rcScreenRect.top= m_rcScreenRect.left=0;
	m_rcScreenRect.right=Width;
	m_rcScreenRect.bottom=Height;
	

	if(CreateDeviceAndSwapChain()){return;}
	
	CreateShader();
	if(CreateRasterizer()){return;}
	CreateViewport();
	CreateDepthStencilBuffer();
	m_pDeviceContext->OMSetRenderTargets(1,&m_pRenderTargetView,m_pDepthStencilView);

}

//----------------------------------------------------------------------------
void CGraphicsLayer::CreateDepthStencilBuffer()
{

	HRESULT r=0;
	D3D11_TEXTURE2D_DESC DescDepth;
	ZeroMemory(&DescDepth,sizeof(DescDepth));
	DescDepth.Width=m_rcScreenRect.right;
	DescDepth.Height=m_rcScreenRect.bottom;
	DescDepth.MipLevels=1;
	DescDepth.ArraySize=1;
	DescDepth.Format=DXGI_FORMAT_D24_UNORM_S8_UINT;
	DescDepth.SampleDesc.Count=1;
	DescDepth.SampleDesc.Quality=0;
	DescDepth.Usage=D3D11_USAGE_DEFAULT;
	DescDepth.BindFlags=D3D11_BIND_DEPTH_STENCIL;
	DescDepth.CPUAccessFlags=0;
	DescDepth.MiscFlags=0;
	//create buffer
	
	r=m_pDevice->CreateTexture2D(&DescDepth,NULL,&m_pDepthStencilBuffer);

	D3D11_DEPTH_STENCIL_DESC DescDS;
	ZeroMemory(&DescDS,sizeof(DescDS));
	DescDS.DepthEnable=true;
	

	DescDS.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ALL;
	DescDS.DepthFunc=D3D11_COMPARISON_LESS;
	DescDS.StencilEnable=true;
	

	DescDS.StencilReadMask=(UINT8)0xFFFFFFFF;
	DescDS.StencilWriteMask=(UINT8)0xFFFFFFFF;

	DescDS.FrontFace.StencilFailOp=D3D11_STENCIL_OP_KEEP;
	DescDS.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_INCR;
	DescDS.FrontFace.StencilPassOp=D3D11_STENCIL_OP_KEEP;
	DescDS.FrontFace.StencilFunc=D3D11_COMPARISON_ALWAYS;

	DescDS.BackFace.StencilFailOp=D3D11_STENCIL_OP_KEEP;
	DescDS.BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_DECR;
	DescDS.BackFace.StencilPassOp=D3D11_STENCIL_OP_KEEP;
	DescDS.BackFace.StencilFunc=D3D11_COMPARISON_ALWAYS;

	r=m_pDevice->CreateDepthStencilState(&DescDS,&m_pDepthStencilState);
	if(FAILED(r))
	{
		return;
	}
	//m_pDevice->OMSetDepthStencilState(m_pDepthStencilState,1);
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilState,1);

	D3D11_DEPTH_STENCIL_VIEW_DESC DescDSView;
	ZeroMemory(&DescDSView,sizeof(DescDSView));
	DescDSView.Format=DXGI_FORMAT_D24_UNORM_S8_UINT;
	DescDSView.ViewDimension=D3D11_DSV_DIMENSION_TEXTURE2D;
	DescDSView.Texture2D.MipSlice=0;

	r=m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer,&DescDSView,&m_pDepthStencilView);
	if(FAILED(r))
	{
		return;
	}

}

//-------------------------------------------------------------------------------------------
int  CGraphicsLayer::CreateRasterizer()
{
	HRESULT r=0;

	D3D11_RASTERIZER_DESC RasterDesc;
	
	ZeroMemory(&RasterDesc,sizeof(RasterDesc));
	RasterDesc.FillMode=D3D11_FILL_SOLID;
	RasterDesc.CullMode=D3D11_CULL_BACK;
	RasterDesc.FrontCounterClockwise=FALSE;
	RasterDesc.DepthBias=0;
	RasterDesc.SlopeScaledDepthBias=0.0f;
	RasterDesc.DepthBiasClamp=0.0f;
	RasterDesc.DepthClipEnable=TRUE;
	RasterDesc.MultisampleEnable=FALSE;
	RasterDesc.ScissorEnable=FALSE;
	RasterDesc.AntialiasedLineEnable=FALSE;

	if(m_pDevice->CreateRasterizerState(&RasterDesc,&m_pRasterizerStateCullBack)!=S_OK){return 1;}
	

	RasterDesc.FillMode=D3D11_FILL_SOLID;
	RasterDesc.CullMode=D3D11_CULL_FRONT;
	RasterDesc.FrontCounterClockwise=FALSE;
	RasterDesc.DepthBias=0;
	RasterDesc.SlopeScaledDepthBias=0.0f;
	RasterDesc.DepthBiasClamp=0.0f;
	RasterDesc.DepthClipEnable=TRUE;
	RasterDesc.MultisampleEnable=FALSE;
	RasterDesc.ScissorEnable=FALSE;
	RasterDesc.AntialiasedLineEnable=FALSE;

	if(m_pDevice->CreateRasterizerState(&RasterDesc,&m_pRasterizerStateCullFront)!=S_OK){return 1;}
	

	


	RasterDesc.FillMode=D3D11_FILL_SOLID;
	//RasterDesc.FillMode=D3D11_FILL_WIREFRAME;
	RasterDesc.CullMode=D3D11_CULL_NONE;
	RasterDesc.FrontCounterClockwise=FALSE;
	RasterDesc.DepthBias=0;
	RasterDesc.SlopeScaledDepthBias=0.0f;
	RasterDesc.DepthBiasClamp=0.0f;
	RasterDesc.DepthClipEnable=FALSE;
	RasterDesc.MultisampleEnable=TRUE;
	RasterDesc.ScissorEnable=FALSE;
	RasterDesc.AntialiasedLineEnable=FALSE;
	
	if(m_pDevice->CreateRasterizerState(&RasterDesc,&m_pRasterizerStateCullNone)!=S_OK){return 1;}
	


	
	m_pDeviceContext->RSSetState(m_pRasterizerStateCullBack);

	return 0;
}

//--------------------------------------------------------------------------
int CGraphicsLayer::CreateDeviceAndSwapChain()
{
	HRESULT r=0;
	//adapter
    m_D3D11Adapter = NULL; 
	//HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&m_DXGIFactory) );
	//hr=m_DXGIFactory->MakeWindowAssociation( NULL, 0 );

	//device parameters
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc,sizeof(swapDesc));

	//one back buffer
	swapDesc.BufferCount=1;

	//width and height of back buffer
	swapDesc.BufferDesc.Width=m_rcScreenRect.right;
	swapDesc.BufferDesc.Height=m_rcScreenRect.bottom;
	swapDesc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;

	//60hz refresh
	swapDesc.BufferDesc.RefreshRate.Numerator=60;
	swapDesc.BufferDesc.RefreshRate.Denominator=1;
	swapDesc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
	swapDesc.OutputWindow=m_hWnd;
	swapDesc.SampleDesc.Count=1;
	swapDesc.SampleDesc.Quality=0;
	swapDesc.Windowed=TRUE;


    D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL FeatureLevels[] =
                {
                    D3D_FEATURE_LEVEL_11_0,
                    D3D_FEATURE_LEVEL_10_1,
                    D3D_FEATURE_LEVEL_10_0,
                    D3D_FEATURE_LEVEL_9_3,
                    D3D_FEATURE_LEVEL_9_2,
                    D3D_FEATURE_LEVEL_9_1
                };
    UINT NumFeatureLevels = ARRAYSIZE( FeatureLevels );
    //use ref
	//FeatureLevels[0] = D3D_FEATURE_LEVEL_11_0;
   // NumFeatureLevels = 1;

	
	if(D3D11CreateDeviceAndSwapChain(
										m_D3D11Adapter,
										D3D_DRIVER_TYPE_HARDWARE,
										//D3D_DRIVER_TYPE_REFERENCE,
										( HMODULE )0,
										D3D11_CREATE_DEVICE_DEBUG,
										FeatureLevels,
										NumFeatureLevels,
										D3D11_SDK_VERSION,
										&swapDesc,
										&m_pSwapChain,
										&m_pDevice,
										&FeatureLevel,
										&m_pDeviceContext
										)!=S_OK)
	{
		MessageBox(m_hWnd, TEXT("Failed Create Device"), TEXT("Init Failure"), MB_OK | MB_ICONERROR);
	}




    IDXGIDevice* pDXGIDev = NULL;
    r = m_pDevice->QueryInterface( __uuidof( IDXGIDevice ), ( LPVOID* )&pDXGIDev );
    if( SUCCEEDED( r ) && pDXGIDev )
    {
        pDXGIDev->GetAdapter( &m_D3D11Adapter );
    }
       
		

	//GET BACK BUFFER
	if(m_pSwapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),(LPVOID*)&m_pBackBuffer)!=S_OK){return 1;}
	
	if(m_pDevice->CreateRenderTargetView(m_pBackBuffer,NULL,&m_pRenderTargetView)!=S_OK){return 1;}

	
	return 0;
}
//-------------------------------------------------------------------------------
void CGraphicsLayer::CreateViewport()
{
	D3D11_VIEWPORT vp;
	vp.Width=m_rcScreenRect.right;
	vp.Height=m_rcScreenRect.bottom;
	vp.MinDepth=0.0f;
	vp.MaxDepth=1.0f;
	vp.TopLeftX=0;
	vp.TopLeftY=0;
	m_pDeviceContext->RSSetViewports(1,&vp);
}

//-------------------------------------------------------------------------------
void CGraphicsLayer::Present()
{

	HRESULT r=S_OK;
	
	if(Graphics())
	{
		if( NULL == m_pSwapChain )
        return;
		r=m_pSwapChain->Present(0,0);
	}
	

}

//------------------------------------------------------------------------------
void CGraphicsLayer::ClearBackBuffer()
{
	
	float Col[4]={1.0f,1.0f,1.0f,1.0f};
	//m_pDevice->ClearRenderTargetView(m_pRenderTargetView,Col);
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView,Col);


}
//----------------------------------------------------------------------
void CGraphicsLayer::ClearStencilBuffer()
{
	
	//m_pDevice->ClearDepthStencilView(m_pDepthStencilView, D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0, 0 );
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0 );
}
/*
//---------------------------------------------------------------------------------
void CGraphicsLayer::DestroyAll()
{
	if(m_pRenderTargetView)free(m_pRenderTargetView);
	if(m_pBackBuffer)free(m_pBackBuffer);
	if(m_pSwapChain)free(m_pSwapChain);
	if(m_pDevice)free(m_pDevice);
	m_pGlobalGLayer=NULL;
}

*/
//------------------------------------------------------------------
HRESULT CGraphicsLayer::CreateConstantsBuffer()
{
	HRESULT hr = S_OK;

	CONSTANTS_MATRICES pData;

	// Setup constant buffers
    D3D11_BUFFER_DESC Desc;
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Desc.MiscFlags = 0;
	Desc.ByteWidth = sizeof( CONSTANTS_MATRICES );
	Desc.ByteWidth = 368;
	Desc.StructureByteStride = 0;

	
    
	hr=m_pDevice->CreateBuffer( &Desc, NULL, &m_pConstantsBuffer );

    return hr;


}
//------------------------------------------------------------------------
int CGraphicsLayer::CreateShader()
{

	 HRESULT r=0;

	 DWORD ShaderFlags=D3D10_SHADER_ENABLE_STRICTNESS;

	 ID3D10Blob *pErrors = 0;

	// Compile shaders
	 ID3D10Blob* pBlobVS = NULL;
	 ID3D10Blob* pBlobHS = NULL;
	 ID3D10Blob* pBlobDS = NULL;
	 ID3D10Blob* pBlobPS = NULL;

	 
	 ID3D10Blob* pBlobGS = NULL;

	
	 if(CompileShaderFromFile( L"D3D11SimpleFx.hlsl", "SmoothVS",	   "vs_5_0", &pBlobVS )!=S_OK){return 1;}
	 if(CompileShaderFromFile( L"D3D11SimpleFx.hlsl", "SmoothHS",	   "hs_5_0", &pBlobHS )!=S_OK){return 1;}
	 if(CompileShaderFromFile( L"D3D11SimpleFx.hlsl", "SmoothDS",	   "ds_5_0", &pBlobDS )!=S_OK){return 1;}
	 if(CompileShaderFromFile( L"D3D11SimpleFx.hlsl", "SmoothGS",	   "gs_5_0", &pBlobGS )!=S_OK){return 1;}
	 if(CompileShaderFromFile( L"D3D11SimpleFx.hlsl", "SmoothPS",	   "ps_5_0", &pBlobPS )!=S_OK){return 1;}
	
	 
	 if(m_pDevice->CreateVertexShader( pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, &m_pVSGouraud )!=S_OK){return 1;}
	 if(m_pDevice->CreateHullShader(  pBlobHS->GetBufferPointer(), pBlobHS->GetBufferSize(), NULL, &m_pHSSmooth )!=S_OK){return 1;}
	 if(m_pDevice->CreateDomainShader(  pBlobDS->GetBufferPointer(), pBlobDS->GetBufferSize(), NULL, &m_pDSSmooth )!=S_OK){return 1;}
	 if(m_pDevice->CreateGeometryShader(  pBlobGS->GetBufferPointer(), pBlobGS->GetBufferSize(), NULL, &m_pGSSmooth )!=S_OK){return 1;}
	 if(m_pDevice->CreatePixelShader(  pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), NULL, &m_pPSGouraud )!=S_OK){return 1;}
	 
	 pBlobPS = NULL;
	 //PhongPS
	 if(CompileShaderFromFile( L"D3D11SimpleFx.hlsl", "PhongPS",	   "ps_5_0", &pBlobPS )!=S_OK){return 1;}
	 if(m_pDevice->CreatePixelShader(  pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), NULL, &m_pPSPhong )!=S_OK){return 1;}

	 pBlobPS = NULL;
	 //TexturePS
	 if(CompileShaderFromFile( L"D3D11SimpleFx.hlsl", "TexturePS",	   "ps_5_0", &pBlobPS )!=S_OK){return 1;}
	 if(m_pDevice->CreatePixelShader(  pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), NULL, &m_pPSTexture )!=S_OK){return 1;}

	 pBlobPS = NULL;
	 //UVTexturePS
	 if(CompileShaderFromFile( L"D3D11SimpleFx.hlsl", "UVTexturePS",	   "ps_5_0", &pBlobPS )!=S_OK){return 1;}
	 if(m_pDevice->CreatePixelShader(  pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), NULL, &m_pPSUV )!=S_OK){return 1;}

	 pBlobGS = NULL;
	 if(CompileShaderFromFile( L"D3D11SimpleFx.hlsl", "DispGS",	   "gs_5_0", &pBlobGS )!=S_OK){return 1;}
	 if(m_pDevice->CreateGeometryShader(  pBlobGS->GetBufferPointer(), pBlobGS->GetBufferSize(), NULL, &m_pGSNormal )!=S_OK){return 1;}

	 //Get textures
	 if(D3DX11CreateShaderResourceViewFromFile( m_pDevice, L"Tile.dds",NULL, NULL, &m_TextureTile, NULL )!=S_OK){return 1;};
	 if(D3DX11CreateShaderResourceViewFromFile( m_pDevice, L"disp.dds",NULL, NULL, &m_TextureDisp, NULL )!=S_OK){return 1;};
	 
	 D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory( &sampDesc, sizeof(sampDesc) );
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		if(m_pDevice->CreateSamplerState( &sampDesc, &m_TileSamplerState )!=S_OK){return 1;};

	D3D11_INPUT_ELEMENT_DESC defaultLayout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT   ,0,							  0,D3D11_INPUT_PER_VERTEX_DATA,0},
	};

	if(m_pDevice->CreateInputLayout( defaultLayout, ARRAYSIZE( defaultLayout ), pBlobVS->GetBufferPointer(),
		pBlobVS->GetBufferSize(), &m_pVertexLayout )!=S_OK){return 1;}
	
	m_pDeviceContext->IASetInputLayout(m_pVertexLayout);

	if(CreateConstantsBuffer()!=S_OK){return 1;}



	D3DXMATRIX mtxWorld;
    D3DXMatrixIdentity(&mtxWorld);
	SetWorldMtx(mtxWorld);

	D3DXMATRIX mtxView;
	D3DXVECTOR3 vecEye(3.0f, 3.0f, 3.5f);
    D3DXVECTOR3 vecAt(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vecUp(0.0f, 1.0f, 0.0f);
    D3DXMatrixLookAtLH(&mtxView, &vecEye, &vecAt, &vecUp);
	SetViewMtx(mtxView);

	

    D3DXMatrixPerspectiveFovLH(&m_mProj, (float)D3DX_PI * 0.5f, 
		m_rcScreenRect.right/(float)m_rcScreenRect.bottom, 0.1f, 50.0f);

  

	UpdateMatrices();

}
//---------------------------------------------------
void CGraphicsLayer::UpdateMatrices()
{
	
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    m_pDeviceContext->Map( m_pConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
    CONSTANTS_MATRICES* pData = ( CONSTANTS_MATRICES* )MappedResource.pData;

   	
    D3DXMatrixTranspose( &pData->g_mWorld, &m_mWorld );
	pData->g_mProj=m_mProj;
	D3DXMATRIX mViewProjection;
	mViewProjection = m_mView * m_mProj;
	
    D3DXMatrixTranspose( &pData->g_mView, &mViewProjection );

	pData->g_Ambient = D3DXVECTOR4(0.3, 0.3, 0.3, 1);
	pData->g_Diffuse = D3DXVECTOR4(0.7, 0.7, 0.7, 1);
	pData->g_Specular = D3DXVECTOR4(0.65, 0.65, 0.25, 1);
	pData->g_kDiffuse = 1.0;
	pData->g_PhongExp = 128.0;
	pData->g_vEyePos = D3DXVECTOR3(2,0,2);
	
	pData->g_vLightPos1 = D3DXVECTOR3(0.0, 10.0, 0.0);
	pData->g_vLightPos2 = D3DXVECTOR3(10.0, 0.0, 0.0);
	pData->g_vLightPos3 = D3DXVECTOR3(-10.0, -10.0, 0.0);
	
	pData->g_vLightColor[0] = D3DXVECTOR4(0.6, 0.6, 0.6, 1.0);
	pData->g_vLightColor[1] = D3DXVECTOR4(0.0, 0.2, 0.7, 1.0);
	pData->g_vLightColor[2] = D3DXVECTOR4(1.0, 0.3, 0.0, 1.0);

    m_pDeviceContext->Unmap( m_pConstantsBuffer, 0 );
}

//----------------------------------------------------------------------------------------
ID3D11RasterizerState *CGraphicsLayer::GetRasterizerState()
{
	return m_pRasterizerStateCullBack;
}

//---------------------------------------------------------------------------------
void CGraphicsLayer::SetCullBack()
{
	m_pDeviceContext->RSSetState(m_pRasterizerStateCullBack);

}
//---------------------------------------------------------------------------------
void CGraphicsLayer::SetCullFront()
{
	m_pDeviceContext->RSSetState(m_pRasterizerStateCullFront);
}
//---------------------------------------------------------------------------------
void CGraphicsLayer::SetCullNone()
{
	m_pDeviceContext->RSSetState(m_pRasterizerStateCullNone);
}
void CGraphicsLayer::SetMatrices()
{

	m_pDeviceContext->VSSetConstantBuffers( 0, 1, &m_pConstantsBuffer );
	m_pDeviceContext->DSSetConstantBuffers( 0, 1, &m_pConstantsBuffer );
	m_pDeviceContext->GSSetConstantBuffers( 0, 1, &m_pConstantsBuffer );
	m_pDeviceContext->PSSetConstantBuffers( 0, 1, &m_pConstantsBuffer );
}

void CGraphicsLayer::SetSmooth()
{
	switch(m_techId) {
		case 0:
			m_pDeviceContext->VSSetShader( m_pVSGouraud, NULL, 0 );
			m_pDeviceContext->HSSetShader( m_pHSSmooth, NULL, 0 );
			m_pDeviceContext->DSSetShader( m_pDSSmooth, NULL, 0 );
			m_pDeviceContext->GSSetShader( m_pGSSmooth, NULL, 0 );
			m_pDeviceContext->PSSetShader( m_pPSGouraud, NULL, 0 );
			break;
		case 1:
			m_pDeviceContext->VSSetShader( m_pVSGouraud, NULL, 0 );
			m_pDeviceContext->HSSetShader( m_pHSSmooth, NULL, 0 );
			m_pDeviceContext->DSSetShader( m_pDSSmooth, NULL, 0 );
			m_pDeviceContext->GSSetShader( m_pGSSmooth, NULL, 0 );
			m_pDeviceContext->PSSetShader( m_pPSPhong, NULL, 0 );
			break;
		case 2:
			m_pDeviceContext->VSSetShader( m_pVSGouraud, NULL, 0 );
			m_pDeviceContext->HSSetShader( m_pHSSmooth, NULL, 0 );
			m_pDeviceContext->DSSetShader( m_pDSSmooth, NULL, 0 );
			m_pDeviceContext->GSSetShader( m_pGSSmooth, NULL, 0 );
			m_pDeviceContext->PSSetShader( m_pPSTexture, NULL, 0 );
			m_pDeviceContext->PSSetShaderResources( 0, 1, &m_TextureTile );
			m_pDeviceContext->PSSetSamplers( 0, 1, &m_TileSamplerState );
			break;
		case 3:
			m_pDeviceContext->VSSetShader( m_pVSGouraud, NULL, 0 );
			m_pDeviceContext->HSSetShader( m_pHSSmooth, NULL, 0 );
			m_pDeviceContext->DSSetShader( m_pDSSmooth, NULL, 0 );
			m_pDeviceContext->GSSetShader( m_pGSSmooth, NULL, 0 );
			m_pDeviceContext->PSSetShader( m_pPSUV, NULL, 0 );
			m_pDeviceContext->PSSetShaderResources( 0, 1, &m_TextureTile );
			m_pDeviceContext->PSSetSamplers( 0, 1, &m_TileSamplerState );
			break;
		case 4:
			m_pDeviceContext->VSSetShader( m_pVSGouraud, NULL, 0 );
			m_pDeviceContext->HSSetShader( m_pHSSmooth, NULL, 0 );
			m_pDeviceContext->DSSetShader( m_pDSSmooth, NULL, 0 );
			m_pDeviceContext->GSSetShader( m_pGSNormal, NULL, 0 );
			m_pDeviceContext->PSSetShader( m_pPSPhong, NULL, 0 );
			m_pDeviceContext->GSSetShaderResources( 0, 1, &m_TextureDisp );
			m_pDeviceContext->GSSetSamplers( 0, 1, &m_TileSamplerState );
			break;
	}
}


//-----------------------------------------------------------------------------------------------------------------
HRESULT CGraphicsLayer::CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D10Blob** ppBlobOut )
{
    HRESULT hr = S_OK;

    // find the file
    WCHAR str[MAX_PATH];
    
	HANDLE hFile = CreateFile( L".\\D3D11SimpleFx.hlsl", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                               FILE_FLAG_SEQUENTIAL_SCAN, NULL );

    if( INVALID_HANDLE_VALUE == hFile )
        return E_FAIL;

    // Get the file size
    LARGE_INTEGER FileSize;
    GetFileSizeEx( hFile, &FileSize );

    // create enough space for the file data
    BYTE* pFileData = new BYTE[ FileSize.LowPart ];
    if( !pFileData )
        return E_OUTOFMEMORY;

    // read the data in
    DWORD BytesRead;
    if( !ReadFile( hFile, pFileData, FileSize.LowPart, &BytesRead, NULL ) )
        return E_FAIL; 

    CloseHandle( hFile );

    // Compile the shader
    ID3DBlob* pErrorBlob;
    hr = D3DCompile( pFileData, FileSize.LowPart, "none", NULL, NULL, szEntryPoint, szShaderModel, D3D10_SHADER_ENABLE_STRICTNESS, 0, ppBlobOut, &pErrorBlob );

    delete []pFileData;

    if( FAILED(hr) )
    {
        OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        //SAFE_RELEASE( pErrorBlob );
        return hr;
    }
   // SAFE_RELEASE( pErrorBlob );

    return S_OK;
}

