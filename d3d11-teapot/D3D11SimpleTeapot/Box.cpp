#include "StdAfx.h"
#include "Box.h"
#include "GraphicsLayer.h"
#include <iostream>

CBox::CBox(void)
{
	m_pVertexBuffer=NULL;
	m_pPatchIndexBuffer=NULL;

	m_iNumVertex=0;
	m_iNumPatches=0;
	m_Ang=1.1;
	
	SetVertex();
	SetPatches();
	CreateBuffers();

}
//------------------------------------------
CBox::~CBox(void)
{
}
//------------------------------
int CBox::GetNumVerts(void)
{
	return m_iNumVertex;
}

//--------------------------------------
int CBox::GetNumPatches()
{
	return  m_iNumPatches;
}
HRESULT CBox::CreateBuffers()
{

	HRESULT hr = S_OK;
	D3D11_BUFFER_DESC DescBuffer;
	memset(&DescBuffer,0,sizeof(DescBuffer));
	D3D11_SUBRESOURCE_DATA resData;
	memset(&resData,0,sizeof(resData));

	//Settings for vertex buffer
	DescBuffer.Usage=D3D11_USAGE_DYNAMIC;
	DescBuffer.ByteWidth=sizeof(BoxVertex)*GetNumVerts();
	DescBuffer.BindFlags= D3D11_BIND_VERTEX_BUFFER;
	DescBuffer.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
	DescBuffer.MiscFlags=0;
	resData.pSysMem=&m_BoxVertex[0];
	hr=Graphics()->GetDevice()->CreateBuffer(&DescBuffer,&resData,&m_pVertexBuffer);
	if(hr!=S_OK)return hr;

	
	//Settings for Patch Index Buffer
	DescBuffer.Usage=D3D11_USAGE_DEFAULT;
	DescBuffer.ByteWidth=sizeof(WORD)*GetNumPatches()*16;
	DescBuffer.BindFlags=D3D11_BIND_INDEX_BUFFER;
	DescBuffer.CPUAccessFlags = 0;
	DescBuffer.MiscFlags = 0;
	resData.pSysMem=&m_PatchIndx[0];
	hr=Graphics()->GetDevice()->CreateBuffer(&DescBuffer,&resData,&m_pPatchIndexBuffer);
	

	return S_OK;
}

//-------------------------------------
void CBox::DrawSmooth(void)
{
	//actualize the transformations
	Graphics()->SetWorldMtx(m_mToWorldXFrom);
	Graphics()->SetCullNone();
	Graphics()->SetSmooth();
	
	//get the buffers
	UINT uiStride=sizeof(BoxVertex);
	UINT uiOffset=0;
	Graphics()->GetDeviceContext()->IASetVertexBuffers(0,1,&m_pVertexBuffer,&uiStride,&uiOffset);
	Graphics()->GetDeviceContext()->IASetIndexBuffer(m_pPatchIndexBuffer,DXGI_FORMAT_R16_UINT,0);
	Graphics()->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);
	
	Graphics()->UpdateMatrices();
	Graphics()->SetMatrices();
	Graphics()->GetDeviceContext()->DrawIndexed(GetNumPatches()*16,0,0);

}


void CBox::SetVertex()
{
	
	float x,y,z;

	// Load the vertices
	FILE *file = fopen(".\\TeapotBez.txt", "rt");
	if (!file) 
	{
		return;
	}


	int i=0;
	while (!feof(file)) {
		fscanf(file, "%f %f %f,\n", &x,&y,&z);
		m_BoxVertex[i].m_vPoint.x=x;
		m_BoxVertex[i].m_vPoint.y=y;
		m_BoxVertex[i].m_vPoint.z=z;
		i++;
	}
	fclose(file);
	m_iNumVertex = i;
	


}

void CBox::SetPatches()
{
	
	
	// Load the triangles
	int a0,a1,a2,a3,a4,a5,a6,a7, a8,a9,a10,a11,a12,a13,a14,a15,dummy;
	FILE *file = fopen("TeapotPatch.txt", "rt");
	if (!file) 
	{
		return;
	}

	int i=0;
	while (!feof(file)) {
		fscanf(file, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d , %d, %d, %d, %d, %d \n", 
			&a0, &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10, &a11, &a12, &a13, &a14, &a15, &dummy);
		m_PatchIndx[i].v[0]=a0;
		m_PatchIndx[i].v[1]=a1;
		m_PatchIndx[i].v[2]=a2;
		m_PatchIndx[i].v[3]=a3;
		m_PatchIndx[i].v[4]=a4;
		m_PatchIndx[i].v[5]=a5;
		m_PatchIndx[i].v[6]=a6;
		m_PatchIndx[i].v[7]=a7;
		m_PatchIndx[i].v[8]=a8;
		m_PatchIndx[i].v[9]=a9;
		m_PatchIndx[i].v[10]=a10;
		m_PatchIndx[i].v[11]=a11;
		m_PatchIndx[i].v[12]=a12;
		m_PatchIndx[i].v[13]=a13;
		m_PatchIndx[i].v[14]=a14;
		m_PatchIndx[i].v[15]=a15;
		i++;
	}
	fclose(file);
	m_iNumPatches = i;
	


}
void CBox::Update(float DeltaTime)
{
	m_Ang+=DeltaTime;
	D3DXMatrixRotationYawPitchRoll(&m_mToWorldXFrom,m_Ang,0.1*m_Ang,0.1*m_Ang);
	
}