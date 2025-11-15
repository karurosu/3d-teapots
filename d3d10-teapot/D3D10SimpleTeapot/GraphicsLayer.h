#pragma once
#ifndef CGraphics_LAYER
#define CGraphics_LAYER

class CGraphicsLayer
{

protected:
	CGraphicsLayer(HWND	hwnd);								//constructor

	HWND						 m_hWnd;					//window handler
	ID3D10Device				*m_pDevice;					//direc3d device
	BOOL						 m_bDeviceActive;
	ID3D10VertexShader			*m_pVShader;
	ID3D10PixelShader			*m_pPShader;
	ID3D10Effect				*m_pEffect10;
	float						 m_fRot;						//delete me
	UINT						 m_wire;						//default is solid, wire is false
	int							 m_result;					//0: PASS, 1: FAIL
	bool						 m_pause;
	bool						 m_BSphere;

	ID3D10EffectMatrixVariable	*m_pmWorld;
	ID3D10EffectMatrixVariable	*m_pmView;
	ID3D10EffectMatrixVariable	*m_pmProj;
	ID3D10EffectScalarVariable  *m_pGrassHeight;


	ID3D10EffectShaderResourceVariable *m_pTextures;
	ID3D10EffectShaderResourceVariable *m_pGlossTex;
	ID3D10EffectShaderResourceVariable *m_pCubeMapTex;
	ID3D10EffectShaderResourceVariable *m_pNormalTex;
	ID3D10EffectShaderResourceVariable *m_pAlphaTex;
	ID3D10EffectShaderResourceVariable *m_pGrassTex[3];

	IDXGISwapChain				*m_pSwapChain;
	RECT						 m_rcScreenRect;
	ID3D10RenderTargetView		*m_pRenderTargetView;
	static CGraphicsLayer		*m_pGlobalGLayer;
	ID3D10Texture2D				*m_pBackBuffer;
	ID3D10RasterizerState       *m_pRasterizerStateCullBack;
	ID3D10RasterizerState       *m_pRasterizerStateCullFront;
	ID3D10RasterizerState       *m_pRasterizerStateCullNone;
	ID3D10RasterizerState       *m_pRasterizerStateWireframe;
								

	//NEW
	ID3D10EffectTechnique		*m_pDrawTech;
	ID3D10EffectTechnique		*m_pDrawTechPhong;
	ID3D10EffectTechnique		*m_pDrawTechDiffuse;
	ID3D10EffectTechnique		*m_pDrawTechGloss;
	ID3D10EffectTechnique		*m_pDrawTechNormal;
	ID3D10EffectTechnique		*m_pDrawTechAlpha;
	ID3D10EffectTechnique		*m_pDrawTechGrass;
	ID3D10EffectTechnique		*m_pDrawTechGrassFans;
	ID3D10EffectTechnique		*m_pDrawTechGrassShells;
	ID3D10EffectTechnique		*m_pDrawTechMatAlpha;
	ID3D10EffectTechnique		*m_pDrawTechReflection;
	ID3D10EffectTechnique		*m_pDrawTechHair;

	ID3D10InputLayout			*m_pVertexLayout;
	ID3D10Texture2D				*m_pDepthStencilBuffer;
	ID3D10DepthStencilState		*m_pDepthStencilState;
	ID3D10DepthStencilView		*m_pDepthStencilView;

	D3DXMATRIXA16				 mWorld;
	D3DXMATRIXA16				 mProj;
	D3DXMATRIXA16				 mView;
	
	int							 m_TechIndx;

public:
	
	void DestroyAll();
	~CGraphicsLayer(void);
	void InitD3D(int Width,int Height,int bpp);
	ID3D10Device *GetDevice()
	{
		return m_pDevice;
	}
	ID3D10Texture2D	*GetBackBuffer()
	{
		return	m_pBackBuffer;
	}
	int Width() const
	{
		return m_rcScreenRect.right;
	}
	int Height() const
	{
		return m_rcScreenRect.bottom;
	}

	void Present();
	void ClearBackBuffer();

	static CGraphicsLayer *GetGraphics()
	{
		return m_pGlobalGLayer;
	}	
	static void Create(HWND hWnd,short width,short height);
	
	void SetWorldMtx(const D3DXMATRIX& mtxWorld){mWorld = mtxWorld;}
	void SetViewMtx(const D3DXMATRIX& mtxView){mView = mtxView;}
	void SetProjMtx(const D3DXMATRIX& mtxProj){mProj = mtxProj;}


	void CreateDepthStencilBuffer();
	void CreateDeviceAndSwapChain();
	void CreateViewport();
	void CreateShader();
	void ClearStencilBuffer();
	void CreateRasterizer();
	ID3D10EffectTechnique *GetTechnique();
	ID3D10EffectTechnique *GetTechnique(int iIndx);
	ID3D10RasterizerState *GetRasterizerState();
	void UpdateMatrices();
	void SetTechnique(int TechIndx);
	int  GetTechniqueIndx();
	void SetTexture(ID3D10ShaderResourceView *pTextureShaderView);
	void SetGlossTexture(ID3D10ShaderResourceView *pTextureShaderView);
	void SetNormalTexture(ID3D10ShaderResourceView *pTextureShaderView);
	void SetAlphaTexture(ID3D10ShaderResourceView *pTextureShaderView);
	void SetCubeMapTexture(ID3D10ShaderResourceView *pTextureShaderView);
	void SetGrassTexture(ID3D10ShaderResourceView *pTextureShaderView,int iIndx);
	void SetCullBack();
	void SetCullFront();
	void SetCullNone();
	void SetWireframe();
	void SetGrassHeight(float Height);


};
CGraphicsLayer *Graphics();
#endif