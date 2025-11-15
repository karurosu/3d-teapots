#include "StdAfx.h"
//#include "Texture.h"
#include "Teapot.h"
#include "GraphicsLayer.h"

#include <iostream>

CTeapot::CTeapot(int parent,float scale,D3DXVECTOR3 p,int iMapType)
{
	
	m_pVertexBuffer=NULL;
	m_pIndexBuffer=NULL;

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

	HRESULT hr = S_OK;
	m_MapType=iMapType;
	SetVertex(scale,iMapType);

	D3D11_BUFFER_DESC DescBuffer;
	memset(&DescBuffer,0,sizeof(DescBuffer));
	DescBuffer.Usage=D3D11_USAGE_DYNAMIC;
	DescBuffer.ByteWidth=sizeof(TeapotVertex)*NumVerts();
	DescBuffer.BindFlags= D3D11_BIND_VERTEX_BUFFER;
	DescBuffer.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
	DescBuffer.MiscFlags=0;

//--------------
	D3D11_SUBRESOURCE_DATA resData;
	memset(&resData,0,sizeof(resData));
	resData.pSysMem=&vertex[0];
	hr=Graphics()->GetDevice()->CreateBuffer(&DescBuffer,&resData,&m_pVertexBuffer);
	

	DescBuffer.Usage=D3D11_USAGE_DEFAULT;
	DescBuffer.ByteWidth=sizeof(WORD)*NumTriangles()*3;
	DescBuffer.BindFlags=D3D11_BIND_INDEX_BUFFER;
	DescBuffer.CPUAccessFlags = 0;
	DescBuffer.MiscFlags = 0;
	resData.pSysMem=&tIndx[0];
	hr=Graphics()->GetDevice()->CreateBuffer(&DescBuffer,&resData,&m_pIndexBuffer);
}
//---------------------------------------------------------------------------------
CTeapot::~CTeapot(void)
{
	//free(m_pTextureDiffuse);
	//free(m_pTextureGloss);
	//free(m_pTextureNormal);
	//free(m_pTextureAlpha);
	//free(m_pTextureGrass[0]);
	//free(m_pTextureGrass[1]);
	//free(m_pTextureGrass[2]);
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
	//actualize the transformations
	Graphics()->SetWorldMtx(m_toWorldXFrom);
	
	//Graphics()->SetGrassHeight(m_GrassHeight);
/*
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
	*/


	//get the buffers
	UINT uiStride=sizeof(TeapotVertex);
	UINT uiOffset=0;
	Graphics()->GetDeviceContext()->IASetVertexBuffers(0,1,&m_pVertexBuffer,&uiStride,&uiOffset);
	Graphics()->GetDeviceContext()->IASetIndexBuffer(m_pIndexBuffer,DXGI_FORMAT_R16_UINT,0);
	
	Graphics()->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//Graphics()->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	Graphics()->SetSmooth();
	Graphics()->UpdateMatrices();
	Graphics()->SetMatrices();
	Graphics()->GetDeviceContext()->DrawIndexed(NumTriangles()*3,0,0);

	
       
     

}/*
void CSkybox11::D3D11Render( D3DXMATRIX* pmWorldViewProj, ID3D11DeviceContext* pd3dImmediateContext )
{
    HRESULT hr;
    
    pd3dImmediateContext->IASetInputLayout( m_pVertexLayout11 );

    UINT uStrides = sizeof( SKYBOX_VERTEX );
    UINT uOffsets = 0;
    ID3D11Buffer* pBuffers[1] = { m_pVB11 };
    pd3dImmediateContext->IASetVertexBuffers( 0, 1, pBuffers, &uStrides, &uOffsets );
    pd3dImmediateContext->IASetIndexBuffer( NULL, DXGI_FORMAT_R32_UINT, 0 );
    pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    pd3dImmediateContext->VSSetShader( m_pVertexShader, NULL, 0 );
    pd3dImmediateContext->PSSetShader( m_pPixelShader, NULL, 0 );

    D3D11_MAPPED_SUBRESOURCE MappedResource;
    V( pd3dImmediateContext->Map( m_pcbVSPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    CB_VS_PER_OBJECT* pVSPerObject = ( CB_VS_PER_OBJECT* )MappedResource.pData;  
    D3DXMatrixInverse( &pVSPerObject->m_WorldViewProj, NULL, pmWorldViewProj );
    pd3dImmediateContext->Unmap( m_pcbVSPerObject, 0 );
    pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &m_pcbVSPerObject );

    pd3dImmediateContext->PSSetSamplers( 0, 1, &m_pSam );
    pd3dImmediateContext->PSSetShaderResources( 0, 1, &m_pEnvironmentRV11 );

    ID3D11DepthStencilState* pDepthStencilStateStored11 = NULL;
    UINT StencilRef;
    pd3dImmediateContext->OMGetDepthStencilState( &pDepthStencilStateStored11, &StencilRef );
    pd3dImmediateContext->OMSetDepthStencilState( m_pDepthStencilState11, 0 );

    pd3dImmediateContext->Draw( 4, 0 );

    pd3dImmediateContext->OMSetDepthStencilState( pDepthStencilStateStored11, StencilRef );
}
*/

void CTeapot::SetVertex(float scale,int iMapType)
{
	LoadVertexFromFile(scale);
	LoadNormalsFromFile();
	LoadIndFromFile();
//	LoadTextures();
	ComputeMapping(iMapType);
}
/*
//-----------------------------------------------------------
void CTeapot::LoadControlPoints( cFile& file )
{
	// load ourselves from point in the file passed in.
	int pointIndex;

	TCHAR buff[1024];
	
	for( pointIndex=0; pointIndex<4; pointIndex++ )
	{
		file.ReadNonCommentedLine( buff, '#' );
		swscanf_s( buff, L"%f %f %f %f %f %f %f %f %f %f %f %f",
			&m_ctrlPoints[pointIndex * 4 + 0].x,
			&m_ctrlPoints[pointIndex * 4 + 0].y,
			&m_ctrlPoints[pointIndex * 4 + 0].z,
			&m_ctrlPoints[pointIndex * 4 + 1].x,
			&m_ctrlPoints[pointIndex * 4 + 1].y,
			&m_ctrlPoints[pointIndex * 4 + 1].z,
			&m_ctrlPoints[pointIndex * 4 + 2].x,
			&m_ctrlPoints[pointIndex * 4 + 2].y,
			&m_ctrlPoints[pointIndex * 4 + 2].z,
			&m_ctrlPoints[pointIndex * 4 + 3].x,
			&m_ctrlPoints[pointIndex * 4 + 3].y,
			&m_ctrlPoints[pointIndex * 4 + 3].z);
	}

	// scale everything up a bit.
	for( int i=0; i<16;i++ )
	{
		m_ctrlPoints[i] = m_ctrlPoints[i] * 10.f;
	}

	// now we can initialize and tesselate the patch
	//Init( 10 );
	//Tesselate();
}
*/

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
/*
//----------------------------------------------------------------------
void CTeapot::LoadTextures()
{
	m_pTextureDiffuse = new CTexture( L".\\LRB_Tile.dds", 0 ); 
	ID3D10ShaderResourceView* pViewDiffuse;
	pViewDiffuse = m_pTextureDiffuse->GetShaderView();
	Graphics()->SetTexture(pViewDiffuse);

	m_pTextureGloss =new CTexture( L".\\LRB_Tile_Gloss.dds", 0 );
	ID3D10ShaderResourceView* pViewGloss;
	pViewGloss = m_pTextureGloss->GetShaderView();
	Graphics()->SetGlossTexture(pViewGloss);

	m_pTextureNormal =new CTexture( L".\\LRB_Tile_Normal.dds", 0 );
	ID3D10ShaderResourceView* pViewNormal;
	pViewNormal = m_pTextureNormal->GetShaderView();
	Graphics()->SetNormalTexture(pViewNormal);

	m_pTextureAlpha =new CTexture( L".\\LRB_TileA.dds", 0 );
	ID3D10ShaderResourceView* pViewAlpha;
	pViewAlpha = m_pTextureAlpha->GetShaderView();
	Graphics()->SetAlphaTexture(pViewAlpha);

	//for grass
	m_pTextureGrass[0] =new CTexture( L".\\Textures\\GrassTex2.dds", 0 );
	ID3D10ShaderResourceView* pViewGrass0;
	pViewGrass0 = m_pTextureGrass[0]->GetShaderView();
	Graphics()->SetGrassTexture(pViewGrass0,0);

	m_pTextureGrass[1] =new CTexture( L".\\Textures\\Grassfins4.dds", 0 );
	ID3D10ShaderResourceView* pViewGrass1;
	pViewGrass1 = m_pTextureGrass[1]->GetShaderView();
	Graphics()->SetGrassTexture(pViewGrass1,1);

	m_pTextureGrass[2] =new CTexture( L".\\Textures\\GrassShell.dds", 0 );
	ID3D10ShaderResourceView* pViewGrass2;
	pViewGrass2 = m_pTextureGrass[2]->GetShaderView();
	Graphics()->SetGrassTexture(pViewGrass2,2);
}
*/
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
void CTeapot::Actualize(float DeltaTime)
{
	m_Ang+=m_Vel*DeltaTime;
	m_ExtAng+=m_ExtVel*DeltaTime;
	m_GrassHeight+=m_Dir*m_GrassVel*DeltaTime;
	if(m_GrassHeight>0.15/m_scale){m_Dir=-1.0;}
	if(m_GrassHeight<0.01/m_scale){m_Dir=1.0;}
}
