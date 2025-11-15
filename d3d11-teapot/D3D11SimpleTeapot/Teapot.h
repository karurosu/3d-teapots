#pragma once
#ifndef CTEAPOT_OBJ
#define CTEAPOT_OBJ
//#include "Texture.h"
//----------------------------------------------------

class TeapotVertex
{
public:
    D3DXVECTOR3 point;
    D3DXVECTOR3 norm;
	D3DXCOLOR	m_vColor;
    D3DXVECTOR2 tcoord;
};
//---------------------------------------------------
class Triangle 
{

public:
    WORD v[3]; // Array access useful in for loops
};
//----------------------------------------------------
class CTeapot
{

public:
	CTeapot(int parent,float scale,D3DXVECTOR3 p,int iMapType);
	~CTeapot(void);

	D3DXVECTOR3					m_Tp;						//global position of the object
	float						m_Ang;						//the rotation angle
	float						m_Vel;						//the rotation velocity
	float						m_ExtVel;					//extra velocity
	float						m_scale;
	float						m_GrassHeight;
	float						m_GrassVel;
	float						m_Dir;

	TeapotVertex				vertex[600];
	Triangle					tIndx[1050];
	ID3D11Buffer				*m_pVertexBuffer;
	ID3D11Buffer				*m_pIndexBuffer;

	D3DXMATRIX					m_toParentXFrom;			//transformation relative to the parent
	D3DXMATRIX					m_toWorldXFrom;				//transformation related to the world coordinates
	int							m_ParentIdx;				//index of the parent solar object(parent teapot)
	D3DXMATRIXA16				mWorldView;					//transformation to the camera's frame
	D3DXMATRIXA16				mWorldViewProjection;		//3D->2D projection
	float						m_ExtAng;						//rotation in (x,y,z)
	int							iNumVertex;
	int							iNumTriangles;
//	CTexture					*m_pTextureDiffuse;
//	CTexture					*m_pTextureGloss;
//	CTexture					*m_pTextureNormal;
//	CTexture					*m_pTextureAlpha;
//	CTexture					*m_pTextureGrass[3];

	int							m_MapType;


	void	 SetVertex(float scale,int iMapType);
	int		 NumVerts(void);
	void	 Draw();
	void	 LoadVertexFromFile(float scale);
	void	 LoadIndFromFile();
	int		 NumTriangles();
	void	 LoadNormalsFromFile();
//	void	 LoadTextures();
	void	 ComputeMapping(int MapType);
	void	 Actualize(float DeltaTime);
};

#endif