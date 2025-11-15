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
	m_pDevice = NULL;					//direc3d device
	m_bDeviceActive=FALSE;

	

	m_pVShader=NULL;
	m_pPShader=NULL;
	m_pEffect10=NULL;
	m_fRot=0;						//delete me
	m_pTextures=NULL;
	m_pGlossTex=NULL;
	m_pNormalTex=NULL;
	m_pAlphaTex=NULL;
	m_pGrassTex[0]=NULL;
	m_pGrassTex[1]=NULL;
	m_pGrassTex[2]=NULL;
	m_pGrassHeight=NULL;
	
	m_wire=0;						//default is solid, wire is false
	m_result=0;					//0: PASS, 1: FAIL
	m_pause=false;
	m_BSphere=false;

	m_pmWorld=NULL;//new
	m_pmView=NULL;//new
	m_pmProj=NULL;//new
	//m_pmWorldView=NULL;//new
	//m_pmWorldViewProjection=NULL;//new

	//from the book
	m_pSwapChain = NULL;
	m_pRenderTargetView = NULL;
	m_pGlobalGLayer = this;
	m_pBackBuffer =NULL;
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
	m_TechIndx=0;

	CreateDeviceAndSwapChain();
	
	CreateRasterizer();
	CreateViewport();
	CreateDepthStencilBuffer();
	m_pDevice->OMSetRenderTargets(1,&m_pRenderTargetView,m_pDepthStencilView);
	
	CreateShader();
	

}
//----------------------------------------------------------------------------
void CGraphicsLayer::CreateDepthStencilBuffer()
{

	HRESULT r=0;
	D3D10_TEXTURE2D_DESC DescDepth;
	ZeroMemory(&DescDepth,sizeof(DescDepth));
	DescDepth.Width=m_rcScreenRect.right;
	DescDepth.Height=m_rcScreenRect.bottom;
	DescDepth.MipLevels=1;
	DescDepth.ArraySize=1;
	DescDepth.Format=DXGI_FORMAT_D24_UNORM_S8_UINT;
	DescDepth.SampleDesc.Count=1;
	DescDepth.SampleDesc.Quality=0;
	DescDepth.Usage=D3D10_USAGE_DEFAULT;
	DescDepth.BindFlags=D3D10_BIND_DEPTH_STENCIL;
	DescDepth.CPUAccessFlags=0;
	DescDepth.MiscFlags=0;
	//create buffer
	r=m_pDevice->CreateTexture2D(&DescDepth,NULL,&m_pDepthStencilBuffer);


	D3D10_DEPTH_STENCIL_DESC DescDS;
	ZeroMemory(&DescDS,sizeof(DescDS));
	DescDS.DepthEnable=true;
	//DescDS.DepthEnable=false;
	DescDS.DepthWriteMask=D3D10_DEPTH_WRITE_MASK_ALL;
	DescDS.DepthFunc=D3D10_COMPARISON_LESS;

	DescDS.StencilEnable=true;
	DescDS.StencilReadMask=(UINT8)0xFFFFFFFF;
	DescDS.StencilWriteMask=(UINT8)0xFFFFFFFF;

	DescDS.FrontFace.StencilFailOp=D3D10_STENCIL_OP_KEEP;
	DescDS.FrontFace.StencilDepthFailOp=D3D10_STENCIL_OP_INCR;
	DescDS.FrontFace.StencilPassOp=D3D10_STENCIL_OP_KEEP;
	DescDS.FrontFace.StencilFunc=D3D10_COMPARISON_ALWAYS;

	DescDS.BackFace.StencilFailOp=D3D10_STENCIL_OP_KEEP;
	DescDS.BackFace.StencilDepthFailOp=D3D10_STENCIL_OP_DECR;
	DescDS.BackFace.StencilPassOp=D3D10_STENCIL_OP_KEEP;
	DescDS.BackFace.StencilFunc=D3D10_COMPARISON_ALWAYS;

	r=m_pDevice->CreateDepthStencilState(&DescDS,&m_pDepthStencilState);
	if(FAILED(r))
	{
		return;
	}
	m_pDevice->OMSetDepthStencilState(m_pDepthStencilState,1);

	D3D10_DEPTH_STENCIL_VIEW_DESC DescDSView;
	ZeroMemory(&DescDSView,sizeof(DescDSView));
	DescDSView.Format=DXGI_FORMAT_D24_UNORM_S8_UINT;
	DescDSView.ViewDimension=D3D10_DSV_DIMENSION_TEXTURE2D;
	DescDSView.Texture2D.MipSlice=0;

	r=m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer,&DescDSView,&m_pDepthStencilView);
	if(FAILED(r))
	{
		return;
	}

}
//-------------------------------------------------------------------------------------------
void CGraphicsLayer::CreateRasterizer()
{
	HRESULT r=0;

	D3D10_RASTERIZER_DESC RasterDesc;
	ZeroMemory(&RasterDesc,sizeof(RasterDesc));
	RasterDesc.FillMode=D3D10_FILL_SOLID;
	RasterDesc.CullMode=D3D10_CULL_BACK;
	RasterDesc.FrontCounterClockwise=FALSE;
	RasterDesc.DepthBias=0;
	RasterDesc.SlopeScaledDepthBias=0.0f;
	RasterDesc.DepthBiasClamp=0.0f;
	RasterDesc.DepthClipEnable=TRUE;
	RasterDesc.MultisampleEnable=FALSE;
	RasterDesc.ScissorEnable=FALSE;
	RasterDesc.AntialiasedLineEnable=FALSE;

	r=m_pDevice->CreateRasterizerState(&RasterDesc,&m_pRasterizerStateCullBack);
	if(FAILED(r))
	{
		return;
	}

	RasterDesc.FillMode=D3D10_FILL_SOLID;
	RasterDesc.CullMode=D3D10_CULL_FRONT;
	RasterDesc.FrontCounterClockwise=FALSE;
	RasterDesc.DepthBias=0;
	RasterDesc.SlopeScaledDepthBias=0.0f;
	RasterDesc.DepthBiasClamp=0.0f;
	RasterDesc.DepthClipEnable=TRUE;
	RasterDesc.MultisampleEnable=FALSE;
	RasterDesc.ScissorEnable=FALSE;
	RasterDesc.AntialiasedLineEnable=FALSE;

	r=m_pDevice->CreateRasterizerState(&RasterDesc,&m_pRasterizerStateCullFront);
	if(FAILED(r))
	{
		return;
	}



	RasterDesc.FillMode=D3D10_FILL_SOLID;
	RasterDesc.CullMode=D3D10_CULL_NONE;
	RasterDesc.FrontCounterClockwise=FALSE;
	RasterDesc.DepthBias=0;
	RasterDesc.SlopeScaledDepthBias=0.0f;
	RasterDesc.DepthBiasClamp=0.0f;
	RasterDesc.DepthClipEnable=FALSE;
	RasterDesc.MultisampleEnable=TRUE;
	RasterDesc.ScissorEnable=FALSE;
	RasterDesc.AntialiasedLineEnable=FALSE;
	
	r=m_pDevice->CreateRasterizerState(&RasterDesc,&m_pRasterizerStateCullNone);
	if(FAILED(r))
	{
		return;
	}

	RasterDesc.FillMode=D3D10_FILL_WIREFRAME;
	RasterDesc.CullMode=D3D10_CULL_BACK;
	RasterDesc.FrontCounterClockwise=FALSE;
	RasterDesc.DepthBias=0;
	RasterDesc.SlopeScaledDepthBias=0.0f;
	RasterDesc.DepthBiasClamp=0.0f;
	RasterDesc.DepthClipEnable=TRUE;
	RasterDesc.MultisampleEnable=FALSE;
	RasterDesc.ScissorEnable=FALSE;
	RasterDesc.AntialiasedLineEnable=FALSE;

	r=m_pDevice->CreateRasterizerState(&RasterDesc,&m_pRasterizerStateWireframe);
	if(FAILED(r))
	{
		return;
	}


	m_pDevice->RSSetState(m_pRasterizerStateCullBack);
}
//--------------------------------------------------------------------------
void CGraphicsLayer::CreateDeviceAndSwapChain()
{
	HRESULT r=0;

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
	swapDesc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;

	swapDesc.OutputWindow=m_hWnd;
	swapDesc.SampleDesc.Count=1;
	swapDesc.SampleDesc.Quality=0;

	swapDesc.Windowed=TRUE;
	r=D3D10CreateDeviceAndSwapChain(	NULL,
									D3D10_DRIVER_TYPE_HARDWARE,
									NULL,
									D3D10_CREATE_DEVICE_DEBUG,
									D3D10_SDK_VERSION,
									&swapDesc,
									&m_pSwapChain,
									&m_pDevice);
	if(FAILED(r))
	{
		//error
		return;
	}


	r=m_pSwapChain->GetBuffer(0,__uuidof(ID3D10Texture2D),(LPVOID*)&m_pBackBuffer);
	
	if(FAILED(r))
	{
		//error
		return;
	}

	r=m_pDevice->CreateRenderTargetView(m_pBackBuffer,NULL,&m_pRenderTargetView);

	
	if(FAILED(r))
	{
		//error
		return;
	}
}
//-------------------------------------------------------------------------------
void CGraphicsLayer::CreateViewport()
{
	D3D10_VIEWPORT vp;
	vp.Width=m_rcScreenRect.right;
	vp.Height=m_rcScreenRect.bottom;
	vp.MinDepth=0.0f;
	vp.MaxDepth=1.0f;
	vp.TopLeftX=0;
	vp.TopLeftY=0;
	m_pDevice->RSSetViewports(1,&vp);
}
//-------------------------------------------------------------------------------
void CGraphicsLayer::Present()
{

	HRESULT r=S_OK;
	//assert(m_pDevice);
	r=m_pSwapChain->Present(0,0);
	if(FAILED(r))
	{
		//error
		return;
	}


}
//------------------------------------------------------------------------------
void CGraphicsLayer::ClearBackBuffer()
{
	HRESULT r=S_OK;
	float Col[4]={1.0f,1.0f,1.0f,1.0f};
	m_pDevice->ClearRenderTargetView(m_pRenderTargetView,Col);

}
//----------------------------------------------------------------------
void CGraphicsLayer::ClearStencilBuffer()
{

	m_pDevice->ClearDepthStencilView(m_pDepthStencilView, D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0, 0 );
}
//---------------------------------------------------------------------------------
void CGraphicsLayer::DestroyAll()
{
	if(m_pRenderTargetView)free(m_pRenderTargetView);
	if(m_pBackBuffer)free(m_pBackBuffer);
	if(m_pSwapChain)free(m_pSwapChain);
	if(m_pDevice)free(m_pDevice);
	m_pGlobalGLayer=NULL;
}


void CGraphicsLayer::CreateShader()
{

 HRESULT r=0;

 DWORD ShaderFlags=D3D10_SHADER_ENABLE_STRICTNESS;

 ID3D10Blob *pErrors = 0;

 r=D3DX10CreateEffectFromFile(L".\\D3D10TeapotTechDiffTexture.fx",
								NULL,
								NULL,
								"fx_4_0",
								ShaderFlags,
								0,
								m_pDevice,
								NULL,
								NULL,
								&m_pEffect10,
								&pErrors,
								NULL);


if(FAILED(r))
{
	char *pCompileError=static_cast<char*>(pErrors->GetBufferPointer());
	SIZE_T errsize = pErrors->GetBufferSize()+1;
	TCHAR *Err = (TCHAR *)malloc(sizeof(TCHAR) * errsize);
	mbstowcs(Err,pCompileError,errsize);
	OutputDebugString(Err);
	free(Err);
	return;
}

	m_pDrawTech				= m_pEffect10->GetTechniqueByName("RenderScene_Gouraud");
	m_pDrawTechPhong		= m_pEffect10->GetTechniqueByName("RenderScene_phong");
	m_pDrawTechDiffuse		= m_pEffect10->GetTechniqueByName("RenderScene_Texture_Diffuse");
	m_pDrawTechGloss		= m_pEffect10->GetTechniqueByName("RenderScene_Texture_Gloss");
	m_pDrawTechNormal		= m_pEffect10->GetTechniqueByName("RenderScene_Texture_Normal");
	m_pDrawTechAlpha		= m_pEffect10->GetTechniqueByName("RenderScene_Texture_Alpha");
	m_pDrawTechGrass		= m_pEffect10->GetTechniqueByName("RenderScene_Texture_Grass");
	m_pDrawTechGrassFans	= m_pEffect10->GetTechniqueByName("RenderScene_Grass_Fans");
	m_pDrawTechGrassShells	= m_pEffect10->GetTechniqueByName("RenderScene_Grass_Shells");
	m_pDrawTechMatAlpha		= m_pEffect10->GetTechniqueByName("RenderScene_Mat_Alpha");
	m_pDrawTechReflection	= m_pEffect10->GetTechniqueByName("RenderScene_reflection");
	m_pDrawTechHair			= m_pEffect10->GetTechniqueByName("RenderScene_hair");


	D3D10_INPUT_ELEMENT_DESC defaultLayout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT   ,0,							  0,D3D10_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL"  ,0,DXGI_FORMAT_R32G32B32_FLOAT   ,0,D3D10_APPEND_ALIGNED_ELEMENT,D3D10_INPUT_PER_VERTEX_DATA,0},
		{"COLOR"   ,0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D10_APPEND_ALIGNED_ELEMENT,D3D10_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT		,0,D3D10_APPEND_ALIGNED_ELEMENT,D3D10_INPUT_PER_VERTEX_DATA,0},
	};

	UINT uiNumElements=sizeof(defaultLayout)/sizeof(defaultLayout[0]);
	D3D10_PASS_DESC descPass;
	m_pDrawTech->GetPassByIndex(0)->GetDesc(&descPass);
	r=m_pDevice->CreateInputLayout(defaultLayout,uiNumElements,descPass.pIAInputSignature,descPass.IAInputSignatureSize,&m_pVertexLayout);
	m_pDevice->IASetInputLayout(m_pVertexLayout);

	//variable relationship
	m_pmWorld  =m_pEffect10->GetVariableByName("g_mWorld")->AsMatrix();
	m_pmView   =m_pEffect10->GetVariableByName("g_mView")->AsMatrix();
	m_pmProj   =m_pEffect10->GetVariableByName("g_mProj")->AsMatrix();
	m_pTextures=m_pEffect10->GetVariableByName("g_DiffuseTex")->AsShaderResource();
	m_pGlossTex=m_pEffect10->GetVariableByName("g_GlossTex")->AsShaderResource();
	m_pNormalTex=m_pEffect10->GetVariableByName("g_NormalTex")->AsShaderResource();
	m_pAlphaTex=m_pEffect10->GetVariableByName("g_AlphaTex")->AsShaderResource();
	m_pCubeMapTex=m_pEffect10->GetVariableByName("g_CubeMapTex")->AsShaderResource();
	m_pGrassTex[0]=m_pEffect10->GetVariableByName("g_GrassSurface")->AsShaderResource();
	m_pGrassTex[1]=m_pEffect10->GetVariableByName("g_GrassFin")->AsShaderResource();
	m_pGrassTex[2]=m_pEffect10->GetVariableByName("g_GrassShell")->AsShaderResource();
	m_pGrassHeight = m_pEffect10->GetVariableByName( "g_GrassHeight" )->AsScalar();


	//m_pmWorldView=m_pEffect10->GetVariableByName("g_mWorldView")->AsMatrix();
	//m_pmWorldViewProjection=m_pEffect10->GetVariableByName("g_mWorldViewProjection")->AsMatrix();
	

	D3DXMATRIX mtxWorld;
    D3DXMatrixIdentity(&mtxWorld);
	SetWorldMtx(mtxWorld);

	D3DXMATRIX mtxView;
	D3DXVECTOR3 vecEye(2.0f, 0.0f, 2.0f);
    D3DXVECTOR3 vecAt(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vecUp(0.0f, 1.0f, 0.0f);
    D3DXMatrixLookAtLH(&mtxView, &vecEye, &vecAt, &vecUp);
	SetViewMtx(mtxView);

	D3DXMATRIX mtxProj;
    D3DXMatrixPerspectiveFovLH(&mtxProj, (float)D3DX_PI * 0.5f, 
		m_rcScreenRect.right/(float)m_rcScreenRect.bottom, 0.1f, 50.0f);
	SetProjMtx(mtxProj);

	UpdateMatrices();

}
//---------------------------------------------------
ID3D10EffectTechnique *CGraphicsLayer::GetTechnique()
{
	switch(m_TechIndx)
	{
		case 0:
			 return m_pDrawTech;
		break;
		case 1:
			return m_pDrawTechPhong;
		break;
		case 2:
			return m_pDrawTechDiffuse;
		break;
		case 3:
			return m_pDrawTechGloss;
		break;
		case 4:
			return m_pDrawTechNormal;
		break;
		case 5:
			return m_pDrawTechAlpha;
		break;
		case 6:
			return m_pDrawTechGrass;
		break;
		case 7:
			return m_pDrawTechGrassFans;
		break;
		case 8:
			return m_pDrawTechGrassShells;
		break;
		case 9:
			return m_pDrawTechMatAlpha;
		break;
		case 10:
			return m_pDrawTechReflection;
		break;
		case 11:
			return m_pDrawTechHair;
		break;
	}
	return m_pDrawTech;
}
//---------------------------------------------------
ID3D10EffectTechnique *CGraphicsLayer::GetTechnique(int iIndx)
{
	switch(iIndx)
	{
		case 0:
			 return m_pDrawTech;
		break;
		case 1:
			return m_pDrawTechPhong;
		break;
		case 2:
			return m_pDrawTechDiffuse;
		break;
		case 3:
			return m_pDrawTechGloss;
		break;
		case 4:
			return m_pDrawTechNormal;
		break;
		case 5:
			return m_pDrawTechAlpha;
		break;
		case 6:
			return m_pDrawTechGrass;
		break;
		case 7:
			return m_pDrawTechGrassFans;
		break;
		case 8:
			return m_pDrawTechGrassShells;
		break;
		case 9:
			return m_pDrawTechMatAlpha;
		break;
		case 10:
			return m_pDrawTechReflection;
		break;
		case 11:
			return m_pDrawTechHair;
		break;
	}
	return m_pDrawTech;
}
//---------------------------------------------------
void CGraphicsLayer::UpdateMatrices()
{
	m_pmWorld->SetMatrix((float*)&mWorld);
    m_pmView->SetMatrix((float*)&mView);
    m_pmProj->SetMatrix((float*)&mProj);
}

//--------------------------------------
void CGraphicsLayer::SetTechnique(int TechIndx)
{
	m_TechIndx=TechIndx;
	return;
}
//------------------------------------------------------------------
int CGraphicsLayer::GetTechniqueIndx()
{
	return m_TechIndx;
}
//---------------------------------------------------------------------------
void CGraphicsLayer::SetTexture(ID3D10ShaderResourceView *pTextureShaderView)
{
	if(m_pTextures)
	{
		m_pTextures->SetResource(pTextureShaderView);
	}
}
//-----------------------------------------------------------------------------------
void CGraphicsLayer::SetGlossTexture(ID3D10ShaderResourceView *pTextureShaderView)
{
	if(m_pGlossTex)
	{
		m_pGlossTex->SetResource(pTextureShaderView);
	}
}
//-----------------------------------------------------------------------------------
void CGraphicsLayer::SetNormalTexture(ID3D10ShaderResourceView *pTextureShaderView)
{
	if(m_pNormalTex)
	{
		m_pNormalTex->SetResource(pTextureShaderView);
	}
}
//-------------------------------------------------------------------------------------
void CGraphicsLayer::SetAlphaTexture(ID3D10ShaderResourceView *pTextureShaderView)
{
	if(m_pAlphaTex)
	{
		m_pAlphaTex->SetResource(pTextureShaderView);
	}
}
//-----------------------------------------------------------------------------------
void CGraphicsLayer::SetCubeMapTexture(ID3D10ShaderResourceView *pTextureShaderView)
{
	if(m_pCubeMapTex)
	{
		m_pCubeMapTex->SetResource(pTextureShaderView);
	}
}
//-------------------------------------------------------------------------------------
void CGraphicsLayer::SetGrassTexture(ID3D10ShaderResourceView *pTextureShaderView,int iIndx)
{
	
	if(m_pGrassTex[iIndx])
		{
			m_pGrassTex[iIndx]->SetResource(pTextureShaderView);
		}
	
}
//----------------------------------------------------------------------------------------
ID3D10RasterizerState *CGraphicsLayer::GetRasterizerState()
{
	return m_pRasterizerStateCullBack;
}
//---------------------------------------------------------------------------------
void CGraphicsLayer::SetCullBack()
{
	m_pDevice->RSSetState(m_pRasterizerStateCullBack);

}
//---------------------------------------------------------------------------------
void CGraphicsLayer::SetWireframe()
{
	m_pDevice->RSSetState(m_pRasterizerStateWireframe);

}
//---------------------------------------------------------------------------------
void CGraphicsLayer::SetCullFront()
{
	m_pDevice->RSSetState(m_pRasterizerStateCullFront);
}
//---------------------------------------------------------------------------------
void CGraphicsLayer::SetCullNone()
{
	m_pDevice->RSSetState(m_pRasterizerStateCullNone);
}
//--------------------------------------------------------------------------------
void CGraphicsLayer::SetGrassHeight(float Height)
{
	m_pGrassHeight->SetFloat(Height);
}