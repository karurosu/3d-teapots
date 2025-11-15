#pragma once
//----------------------------------------------------
class BoxVertex
{
public:
    D3DXVECTOR3 m_vPoint;
};

//---------------------------------------------------
class Patch 
{
public:
    WORD v[16]; 
};

//----------------------------------------------------
class CBox
{
public:
	CBox(void);
	~CBox(void);
	BoxVertex			m_BoxVertex[500];
	Patch				m_PatchIndx[50];

	ID3D11Buffer		*m_pVertexBuffer;
	ID3D11Buffer		*m_pPatchIndexBuffer;

	
	D3DXMATRIX			m_mToWorldXFrom;				//transformation related to the world coordinates
	D3DXMATRIXA16		m_mWorldView;					//transformation to the camera's frame
	D3DXMATRIXA16		m_mWorldViewProjection;		//3D->2D projection
	

	int					m_iNumVertex;
	int					m_iNumPatches;
	float				m_Ang;


	void	 SetVertex();
	void	 SetPatches();

	int		 GetNumVerts();
	int		 GetNumPatches();
	void	 DrawSmooth();
	void	 Update(float DeltaTime);
	HRESULT  CreateBuffers();
};
