#ifndef _VS_CONSOLE
#define _VS_CONSOLE

// To use in Windows Visual Studio
#ifdef USE_WIN_VS
#define print(s) OutputDebugStringA(s)
#endif

// To use in Windows console
#ifdef USE_WIN_ANSI
#define print(s) printf(s)
#endif

#define NO_SHADING	    	    0
#define GOURAUD_SHADING	    	1
#define PHONG_SHADING		    2
#define BLINN_PHONG_SHADING		4
#define SPECULAR_HIGHLIGHT  	8

#define DIFFUSE_TEXTURE     	16 // In HLSL DX9 = 1
#define NORMAL_MAPPING       	32 // In HLSL DX9 = 2
#define REMAP1                  64 // In HLSL DX9 = 4
#define REMAP2                  128 // In HLSL DX9 = 8
#define BEES                    256 // In HLSL DX9 = 16
#define HMAP                    512 // In HLSL DX9 = 32
#define NORMAL_MAPPING2        1024 // In HLSL DX9 = 64

#define E_ORIGINAL          1
#define E_NOROTAR           2
#define E_NUEVO             4
#define E_TRANSPARENCIA     8
#define E_NOROTAR_CTRL     16

/*
Gouraud Shading
	Phong Shading
	Specular highlight
	Diffuse Texture 
	Remap uv’s – 0-2 & 0-1-0
	Align uv’s so 2 complete bees show on side vertically
	Diffuse Texture w/ above
	Diffuse Texture w/ normal mapping
	*/


#endif

