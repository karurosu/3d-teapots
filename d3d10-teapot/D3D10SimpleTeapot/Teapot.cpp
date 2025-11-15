#include "StdAfx.h"
#include "Texture.h"
#include "Teapot.h"
#include "GraphicsLayer.h"

#include <iostream>

CTeapot::CTeapot(int parent,float scale,D3DXVECTOR3 p,int iMapType)
{
	
	m_pVertexBuffer=0;
	iNumVertex=0;
	iNumTriangles=0;
	m_ParentIdx=parent;
	m_Tp=p;
	D3DXMatrixIdentity(&m_toWorldXFrom);
	m_Ang=0;
	m_ExtAng=0;
	m_Vel=0.0003F*scale;
	m_ExtVel=0.0002*scale;
	m_GrassHeight=0.01/scale;
	m_scale=scale;
	m_GrassVel=0.00001/scale;
	m_Dir=1.0;

	m_MapType=iMapType;
	SetVertex(scale,iMapType);
	D3D10_BUFFER_DESC DescBuffer;
	memset(&DescBuffer,0,sizeof(DescBuffer));
	DescBuffer.Usage=D3D10_USAGE_DEFAULT;
	DescBuffer.ByteWidth=sizeof(TeapotVertex)*NumVerts();
	DescBuffer.BindFlags= D3D10_BIND_VERTEX_BUFFER;
	DescBuffer.CPUAccessFlags=0;
	DescBuffer.MiscFlags=0;

	D3D10_SUBRESOURCE_DATA resData;
	memset(&resData,0,sizeof(resData));
	resData.pSysMem=&vertex[0];
	Graphics()->GetDevice()->CreateBuffer(&DescBuffer,&resData,&m_pVertexBuffer);


	DescBuffer.Usage=D3D10_USAGE_DEFAULT;
	DescBuffer.ByteWidth=sizeof(WORD)*NumTriangles()*3;
	DescBuffer.BindFlags=D3D10_BIND_INDEX_BUFFER;
	DescBuffer.CPUAccessFlags = 0;
	DescBuffer.MiscFlags = 0;
	resData.pSysMem=&tIndx[0];
	Graphics()->GetDevice()->CreateBuffer(&DescBuffer,&resData,&m_pIndexBuffer);
}
//---------------------------------------------------------------------------------
CTeapot::~CTeapot(void)
{
	free(m_pTextureDiffuse);
	free(m_pTextureGloss);
	free(m_pTextureNormal);
	free(m_pTextureAlpha);
	free(m_pTextureGrass[0]);
	free(m_pTextureGrass[1]);
	free(m_pTextureGrass[2]);
}
//------------------------------
int CTeapot::NumVerts(void)
{
	return iNumVertex;
}
//--------------------------------------
int CTeapot::NumTriangles()
{
	return  iNumTriangles;
}


//-------------------------------------
void CTeapot::Draw(void)
{
	int i=0;
	//update the transformations
	Graphics()->SetWorldMtx(m_toWorldXFrom);
	Graphics()->UpdateMatrices();
	Graphics()->SetGrassHeight(m_GrassHeight);

	//get the buffers
	UINT uiStride=sizeof(TeapotVertex);
	UINT uiOffset=0;
	Graphics()->GetDevice()->IASetVertexBuffers(0,1,&m_pVertexBuffer,&uiStride,&uiOffset);
	Graphics()->GetDevice()->IASetIndexBuffer(m_pIndexBuffer,DXGI_FORMAT_R16_UINT,0);
	Graphics()->GetDevice()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	//get the technique
	D3D10_TECHNIQUE_DESC DescTechnique;
	Graphics()->GetTechnique()->GetDesc(&DescTechnique);
	
	//case of alpha draw first the back face
	if(Graphics()->GetTechniqueIndx() == 5||Graphics()->GetTechniqueIndx()==9)
	{
		Graphics()->SetCullFront();
		//for(UINT i=0;i<DescTechnique.Passes;i++)
		{
			Graphics()->GetTechnique()->GetPassByIndex(i)->Apply(0);
			Graphics()->GetDevice()->DrawIndexed(NumTriangles()*3,0,0);
		}
	
		Graphics()->SetCullBack();
	}
	

	//Draw the teapot
	//for(UINT i=0;i<DescTechnique.Passes;i++)
	{
		Graphics()->GetTechnique()->GetPassByIndex(i)->Apply(0);
		Graphics()->GetDevice()->DrawIndexed(NumTriangles()*3,0,0);
	}
	
	//Draw Grass
	if(Graphics()->GetTechniqueIndx() == 6)
	{
			
		Graphics()->SetCullNone();
		for(UINT i=0;i<DescTechnique.Passes;i++)
		{
			Graphics()->GetTechnique(7)->GetPassByIndex(i)->Apply(0);
			Graphics()->GetDevice()->DrawIndexed(NumTriangles()*3,0,0);
		}

		Graphics()->SetCullBack();
		//for(UINT i=0;i<DescTechnique.Passes;i++)
		{
			Graphics()->GetTechnique(8)->GetPassByIndex(i)->Apply(0);
			Graphics()->GetDevice()->DrawIndexed(NumTriangles()*3,0,0);
		}	
	}

}


void CTeapot::SetVertex(float scale,int iMapType)
{
	LoadVertexFromFile(scale);
	LoadNormalsFromFile();
	LoadIndFromFile();
	LoadTextures();
	ComputeMapping(iMapType);
}


//-----------------------------------------------------------
void CTeapot::LoadVertexFromFile(float scale)
{
	float x,y,z;

	// Load the vertices
	FILE *file = fopen(".\\Teapot3D.txt", "rt");	
	if (!file) 
	{
		return;
	}


	int i=0;
	while (!feof(file)) {
		fscanf(file, "%f %f %f,\n", &x,&y,&z);
		vertex[i].point.x=x/scale;
		vertex[i].point.y=y/scale;
		vertex[i].point.z=z/scale;
		i++;
	}
	fclose(file);
	iNumVertex = i;
	

}
//---------------------------------------------------------------------
void CTeapot::LoadIndFromFile()
{

	// Load the triangles
	int a,b,c,dummy;
	FILE *file = fopen("TeapotTri.txt", "rt");
	if (!file) 
	{
		return;
	}

	
	int i=0;
	while (!feof(file)) {
		fscanf(file, "%d, %d, %d, %d,\n", &a, &b, &c, &dummy);
		tIndx[i].v[0]=a;
		tIndx[i].v[1]=b;
		tIndx[i].v[2]=c;
		i++;
	}
	fclose(file);
	iNumTriangles = i;
	
	
}
//-----------------------------------------------------------------------------------
void CTeapot::LoadNormalsFromFile()
{
	float nx,ny,nz;

	// Load the vertices
	FILE *file = fopen(".\\TeapotNorm.txt", "rt");	
	if (!file) {
		//MessageBox(NULL, "3D data file not found", NULL, MB_ICONASTERISK | MB_OK );
		return;
	}


	int i=0;
	while (!feof(file)) {
		fscanf(file, "%f %f %f,\n", &nx,&ny,&nz);
		vertex[i].norm.x=nx;
		vertex[i].norm.y=ny;
		vertex[i].norm.z=nz;
		i++;
	}
	fclose(file);
}
//----------------------------------------------------------------------
void CTeapot::LoadTextures()
{
	m_pTextureDiffuse = new CTexture( L".\\Tile.dds", 0 , FALSE); 
	ID3D10ShaderResourceView* pViewDiffuse;
	pViewDiffuse = m_pTextureDiffuse->GetShaderView();
	Graphics()->SetTexture(pViewDiffuse);

	m_pTextureGloss =new CTexture( L".\\Gloss.dds", 0 , FALSE);
	ID3D10ShaderResourceView* pViewGloss;
	pViewGloss = m_pTextureGloss->GetShaderView();
	Graphics()->SetGlossTexture(pViewGloss);

	m_pTextureNormal =new CTexture( L".\\Normal.dds", 0 , FALSE);
	ID3D10ShaderResourceView* pViewNormal;
	pViewNormal = m_pTextureNormal->GetShaderView();
	Graphics()->SetNormalTexture(pViewNormal);

	m_pTextureAlpha =new CTexture( L".\\TileA.dds", 0 , FALSE);
	ID3D10ShaderResourceView* pViewAlpha;
	pViewAlpha = m_pTextureAlpha->GetShaderView();
	Graphics()->SetAlphaTexture(pViewAlpha);

	m_pCubeMap =new CTexture( L".\\skybox.dds", 0 ,TRUE);
	ID3D10ShaderResourceView* pViewCubeMap;
	pViewCubeMap = m_pCubeMap->GetShaderView3D();
	Graphics()->SetCubeMapTexture(pViewCubeMap);

	//for grass
	m_pTextureGrass[0] =new CTexture( L".\\Textures\\GrassTex2.dds", 0 , FALSE);
	ID3D10ShaderResourceView* pViewGrass0;
	pViewGrass0 = m_pTextureGrass[0]->GetShaderView();
	Graphics()->SetGrassTexture(pViewGrass0,0);

	m_pTextureGrass[1] =new CTexture( L".\\Textures\\Grassfins4.dds", 0 , FALSE);
	ID3D10ShaderResourceView* pViewGrass1;
	pViewGrass1 = m_pTextureGrass[1]->GetShaderView();
	Graphics()->SetGrassTexture(pViewGrass1,1);

	m_pTextureGrass[2] =new CTexture( L".\\Textures\\GrassShell.dds", 0 , FALSE);
	ID3D10ShaderResourceView* pViewGrass2;
	pViewGrass2 = m_pTextureGrass[2]->GetShaderView();
	Graphics()->SetGrassTexture(pViewGrass2,2);
}

void CTeapot::ComputeMapping(int MapType)
{
	D3DXVECTOR3 vNorm;
    float theta, phi;// sintheta;
	float u,v;
	m_MapType=MapType;

	for(int i=0;i<iNumVertex;i++)
	{
		
		D3DXVec3Normalize( &vNorm, &vertex[i].point);
        theta = acosf(vNorm.y);
        phi = atan2f(vNorm.x, vNorm.z);

        u = phi / 2.F / D3DX_PI + 0.5F;
		v = theta / D3DX_PI;
		u = (u > 0.5)?(1.F-u):(u);
		switch(MapType)
		{
			case 1:
				v = (v > 0.5)?(1.F-v):(v);
				u /= 2.F;
				v *= 1.F;
			break;
			case 2:
				
				u *= 3.F;
				v *= 3.F;
			break;
		}
		vertex[i].tcoord.x=u;
		vertex[i].tcoord.y=v;
	}

}
//----------------------------------------------------------------
void CTeapot::Update(float DeltaTime)
{
	m_Ang+=m_Vel*DeltaTime;
	m_ExtAng+=m_ExtVel*DeltaTime;
	m_GrassHeight+=m_Dir*m_GrassVel*DeltaTime;
	if(m_GrassHeight>0.15/m_scale){m_Dir=-1.0;}
	if(m_GrassHeight<0.01/m_scale){m_Dir=1.0;}
}
