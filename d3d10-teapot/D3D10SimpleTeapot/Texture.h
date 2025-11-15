#pragma once
#ifndef TEXTURE_HEADER
#define TEXTURE_HEADER


#include <string>

class CTexture
{
protected:
	ID3D10Texture2D			 *m_pTexture;
	ID3D10Texture2D			 *m_pTexture3D;
	bool					 cube;
	ID3D10ShaderResourceView *m_pShaderResourceView;
	std::wstring					  m_name;
	DWORD					  m_stage;


	void ReadDDSTexture(ID3D10Texture2D **pTexture);
	void ReadDDSTexture3D(ID3D10Texture2D **pTexture);
public:
	CTexture(const TCHAR *FileName,DWORD stage, bool use3d);
	~CTexture(void);
	ID3D10Texture2D			 *GetTexture();
	ID3D10Texture2D			 *GetTexture3D();
	ID3D10ShaderResourceView *GetShaderView();
	ID3D10ShaderResourceView *GetShaderView3D();
};


#endif