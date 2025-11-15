#include "StdAfx.h"
#include "Texture.h"
#include "GraphicsLayer.h"


CTexture::CTexture(const TCHAR *FileName,DWORD stage, bool use3d)
{
	m_pShaderResourceView = NULL;
	//ID3D10Texture2D	*pTemp=0;
	m_pTexture = 0;
	m_pTexture3D = 0;
	m_name=std::wstring(FileName);
	m_stage=stage;
	if (use3d != TRUE){
		ReadDDSTexture(&m_pTexture);
	} else {
		ReadDDSTexture3D(&m_pTexture3D);
		cube = use3d;
	}

}
//----------------------------------------------------
CTexture::~CTexture(void)
{


}

//------------------------------------------------
void CTexture::ReadDDSTexture(ID3D10Texture2D **pTexture)
{
	HRESULT r=0;
	r = D3DX10CreateTextureFromFile(Graphics()->GetDevice(),m_name.c_str(),NULL,NULL,(ID3D10Resource**)pTexture,NULL);

}

void CTexture::ReadDDSTexture3D(ID3D10Texture2D **pTexture)
{
	HRESULT r=0;
	D3DX10_IMAGE_LOAD_INFO loadSMInfo;
	loadSMInfo.MiscFlags = D3D10_RESOURCE_MISC_TEXTURECUBE;
	
	r = D3DX10CreateTextureFromFile(Graphics()->GetDevice(),m_name.c_str(),&loadSMInfo,NULL,(ID3D10Resource**)pTexture,NULL);

}
//--------------------------------------------------
ID3D10Texture2D	*CTexture::GetTexture()
{
	return m_pTexture;
}

ID3D10Texture2D	*CTexture::GetTexture3D()
{
	return m_pTexture3D;
}
//--------------------------------------------
ID3D10ShaderResourceView *CTexture::GetShaderView()
{
	if(!m_pShaderResourceView)
	{
		D3D10_TEXTURE2D_DESC DescTexture;
		m_pTexture->GetDesc(&DescTexture);

		D3D10_SHADER_RESOURCE_VIEW_DESC DescShaderView;
		memset(&DescShaderView,0,sizeof(DescShaderView));
		DescShaderView.Format=DescTexture.Format;
		DescShaderView.ViewDimension=D3D10_SRV_DIMENSION_TEXTURE2D;
		DescShaderView.Texture2D.MipLevels=DescTexture.MipLevels;
		HRESULT r;
		r = Graphics()->GetDevice()->CreateShaderResourceView(m_pTexture,&DescShaderView,&m_pShaderResourceView);
	}

	return m_pShaderResourceView;
}

ID3D10ShaderResourceView *CTexture::GetShaderView3D()
{
	HRESULT r;
	if(!m_pShaderResourceView)
	{
		D3D10_RESOURCE_DIMENSION type;
		m_pTexture3D->GetType( &type );

		D3D10_TEXTURE2D_DESC DescTexture;
		m_pTexture3D->GetDesc(&DescTexture);

		D3D10_SHADER_RESOURCE_VIEW_DESC DescShaderView;
		memset(&DescShaderView,0,sizeof(DescShaderView));
		DescShaderView.Format=DescTexture.Format;
		DescShaderView.ViewDimension=D3D11_SRV_DIMENSION_TEXTURECUBE;
		DescShaderView.TextureCube.MipLevels=DescTexture.MipLevels;
		DescShaderView.TextureCube.MostDetailedMip = 0;
		
		r = Graphics()->GetDevice()->CreateShaderResourceView(m_pTexture3D,&DescShaderView,&m_pShaderResourceView);
	}

	return m_pShaderResourceView;
}