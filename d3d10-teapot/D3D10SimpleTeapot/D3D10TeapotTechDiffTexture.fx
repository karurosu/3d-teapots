//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
#define NUM_LIGHTS 3
///////////// UN-TWEAKABLES /////////////

float	 g_GrassHeight=0.01;		 // Grass Height.
float4x4 g_mWorld;                   // World matrix for object
float4x4 g_mView;					 // View matrix for object
float4x4 g_mProj;					 // View matrix for object
float4x4 g_mWorldView;               // World View matrix for object
float4x4 g_mWorldViewProjection;     // World * View * Projection matrix
float3   g_vEyePos = float3(2,0,2);
int g_HairNumber = 10;

//************ Textures *************/


Texture2D g_DiffuseTex;
Texture2D g_GlossTex;
Texture2D g_NormalTex;
Texture2D g_AlphaTex;
Texture2D g_GrassSurface;
Texture2D g_GrassFin;
Texture2D g_GrassShell;
TextureCube g_CubeMapTex;

SamplerState SamplerStateWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

float3  g_vLightPos[NUM_LIGHTS]   = {float3(0, 10, 0),
			 						 float3(10, 0, 0),
                            	     float3(-10, -10, 0)
                            	     //float3(10, 10, -10)
                           			};
                     			
float4  g_vLightColor[NUM_LIGHTS] = {float4(0.6, 0.6, 0.6, 1),
							 	     float4(0, 0.2, 0.7, 1),
									 float4(1, 0.3, 0, 1)
									 //float4(1, 1, 1, 1)
                           			 };
float4 g_Ambient    = float4(0.3, 0.3, 0.3, 1);     // Material's ambient color
float4 g_Diffuse    = float4(0.7, 0.7, 0.7, 1);    // Material's diffuse color
float4 g_Specular   = float4(0.65, 0.65, 0.25, 1);
float  g_kDiffuse   = 1.0f;
float  g_PhongExp   = 128.0f;

///////////// TWEAKABLES /////////////

BlendState NoBlend
{
    BlendEnable[0] = FALSE;
};
BlendState AlphaOn
{
    BlendEnable[0] = TRUE;
    SrcBlend=SRC_ALPHA;
    DestBlend=INV_SRC_ALPHA;
    BlendOp=ADD;
    SrcBlendAlpha=SRC_ALPHA;
    DestBlendAlpha=INV_SRC_ALPHA;
    BlendOpAlpha=ADD;RenderTargetWriteMask[0]=0x0F;
};
DepthStencilState EnableDepthTestingOnly
{
    DepthEnable = true;
    DepthWriteMask = 0x00;
    DepthFunc = Less;
    StencilEnable = false;
};

DepthStencilState EnableDepth
{
    DepthEnable = true;
    DepthWriteMask = ALL;
    DepthFunc = Less;
    StencilEnable = false;
 };
 

//----------------------------------------------------------
struct VS_INPUT
{
	float3 vPosition	:POSITION;
	float3 vNormal		:NORMAL;
	float4 vColor		:COLOR0;
	float2 vTexCoords	:TEXCOORD;
};

struct VS_OUTPUT1
{
	float4 vPosition	:SV_POSITION;
	float4 vColor		:COLOR0;
};

struct VS_OUTPUT2
{
	float4 vPosition	:SV_POSITION;
	float3 vNormal		:NORMAL;
	float4 v3DPos		:POSITION1;
};

struct VS_OUTPUT2_REFLECT
{
	float4 vPosition	:SV_POSITION;
	float3 vNormal		:NORMAL0;
	float4 v3DPos		:POSITION1;
	float3 vReflect		:NORMAL1;
};

struct VS_OUTPUT3
{
	float4 vPosition	:SV_POSITION;
	float3 vNormal		:NORMAL;
	float4 v3DPos		:POSITION1;
	float2 vTexCoords	:TEXCOORD;
};
struct VS_OUTPUT4
{
	float4 vPosition	:SV_POSITION;
	float3 vNormal		:NORMAL;
	float3 vT		:NORMAL1;
	float3 vB		:NORMAL2;
	float4 v3DPos		:POSITION1;
	float2 vTexCoords	:TEXCOORD;
};

//--------------------------------------------------------------------------
// THE Phong Model
//--------------------------------------------------------------------------
float4 PhongModel(float4 Pos, float3 Normal, float4 texColor)
{
	float4 Color = g_Ambient;
	float3 EyeToPos = normalize(Pos.xyz - g_vEyePos);
	float3 LightToPos = float3(0,0,0), Reflected = float3(0,0,0);
	float4 LightPos = float4(2,20,10, 1);
	float DiffuseIntensity  = 0, Spec = 0;
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		LightPos = float4(g_vLightPos[i], 1);
		LightToPos = normalize(Pos.xyz - LightPos.xyz);
		// Compute the diffuse component
		DiffuseIntensity  = saturate(dot(-LightToPos, Normal));
		// Compute the specular component
		Reflected = normalize(LightToPos - 2*dot(Normal, LightToPos) * Normal);
		Spec = saturate(dot(-Reflected, EyeToPos));
		Spec = pow(max(Spec, 0), g_PhongExp);
		Color = Color + g_vLightColor[i] * ((DiffuseIntensity  * texColor) + (Spec * g_Specular));
	}
	return Color;
} 

float4 PhongModelToon(float4 Pos, float3 Normal, float4 texColor, float3x3 tnbMatrix)
{
	float4 Color = g_Ambient;
	float3 LightToPos = float3(0,0,0), Reflected = float3(0,0,0);
	float4 LightPos = float4(2,20,10, 1);

	float3 EyeToPos = normalize(mul(tnbMatrix,(Pos - g_vEyePos)));
	
	float DiffuseIntensity  = 0, Spec = 0;
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		LightPos = float4(g_vLightPos[i], 1);
		LightToPos = normalize(mul(tnbMatrix,(Pos - LightPos)));
		
		// Compute the diffuse component
		DiffuseIntensity  = saturate(dot(-LightToPos, Normal));

		//Truncate
		if (DiffuseIntensity < 0.3) {
			DiffuseIntensity = 0.15;
		} else if (DiffuseIntensity < 0.6) {
			DiffuseIntensity = 0.5;
		} else if (DiffuseIntensity < 0.9) {
			DiffuseIntensity = 0.8;
		} else {
			DiffuseIntensity = 0.9;
		}

		// Compute the specular component
		Reflected = normalize(LightToPos - 2*dot(Normal, LightToPos) * Normal);
		Spec = saturate(dot(-Reflected, EyeToPos));
		Spec = pow(max(Spec, 0), g_PhongExp);

		//Truncate
		if (Spec < 0.3) {
			Spec = 0.2;
		} else if (Spec < 0.6) {
			Spec = 0.5;
		} else if (Spec < 0.9) {
			Spec = 0.8;
		} else {
			Spec = 0.9;
		}

		Color = Color + g_vLightColor[i] * ((DiffuseIntensity * texColor) + (Spec * g_Specular));
	}
	return Color;
} 

float4 PhongModelMap(float4 Pos, float3 Normal, float4 texColor, float3x3 tnbMatrix)
{
	float4 Color = g_Ambient;
	float3 LightToPos = float3(0,0,0), Reflected = float3(0,0,0);
	float4 LightPos = float4(2,20,10, 1);

	float3 EyeToPos = normalize(mul(tnbMatrix,(Pos - g_vEyePos)));
	
	float DiffuseIntensity  = 0, Spec = 0;
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		LightPos = float4(g_vLightPos[i], 1);
		LightToPos = normalize(mul(tnbMatrix,(Pos - LightPos)));
		
		// Compute the diffuse component
		DiffuseIntensity  = saturate(dot(-LightToPos, Normal));
		// Compute the specular component
		Reflected = normalize(LightToPos - 2*dot(Normal, LightToPos) * Normal);
		Spec = saturate(dot(-Reflected, EyeToPos));
		Spec = pow(max(Spec, 0), g_PhongExp);
		Color = Color + g_vLightColor[i] * ((DiffuseIntensity  * texColor) + (Spec * g_Specular));
	}
	return Color;
} 

//--------------------------------------------------------------------------
//GOURAUD
//--------------------------------------------------------------------------
VS_OUTPUT1 VS_GOURAUD(VS_INPUT In)
{	
	VS_OUTPUT1 result;
    
    float4 vPos = float4(In.vPosition, 1.0f);
    float4 vFinalPos = mul(vPos, g_mWorld);
    float4 Pos=vFinalPos;
    vFinalPos = mul(vFinalPos, g_mView);
    vFinalPos = mul(vFinalPos, g_mProj); 
    float3 Normal = normalize( mul( float4(In.vNormal, 0), g_mWorld ) ).xyz;
    result.vColor = PhongModel(Pos, Normal, g_Diffuse);
    result.vPosition = vFinalPos;
    return result;  
}
//----------------------------------------------------------------------------
float4 PS_GOURAUD(VS_OUTPUT1 In) : SV_Target
{
	return In.vColor; 
}

//--------------------------------------------------------------------------
//PHONG
//--------------------------------------------------------------------------
VS_OUTPUT2 VS_PHONG(VS_INPUT In)
{	
	VS_OUTPUT2 result;
    
    float4 vPos = float4(In.vPosition, 1.0f);
    float4 vFinalPos = mul(vPos, g_mWorld);
    result.v3DPos=vFinalPos;
    vFinalPos = mul(vFinalPos, g_mView);
    vFinalPos = mul(vFinalPos, g_mProj); 
    result.vNormal= normalize( mul( float4(In.vNormal, 0), g_mWorld ) ).xyz;
    result.vPosition = vFinalPos;
    return result;  
}
//----------------------------------------------------------------------------
float4 PS_PHONG(VS_OUTPUT2 In) : SV_Target
{
	return PhongModel(In.v3DPos, In.vNormal, g_Diffuse);
}
//-----------------------------------------------------------------------------
//TEXTURE DIFFUSE
//------------------------------------------------------------------------------
VS_OUTPUT3 VS_DIFFUSE(VS_INPUT In)
{	
	VS_OUTPUT3 result;
    
    float4 vPos = float4(In.vPosition, 1.0f);
    float4 vFinalPos = mul(vPos, g_mWorld);
    result.v3DPos=vFinalPos;
    vFinalPos = mul(vFinalPos, g_mView);
    vFinalPos = mul(vFinalPos, g_mProj); 
    result.vNormal= normalize( mul( float4(In.vNormal, 0), g_mWorld ) ).xyz;
    result.vPosition = vFinalPos;
    result.vTexCoords=In.vTexCoords;
    return result;  
}
//----------------------------------------------------------------------------
float4 PS_DIFFUSE(VS_OUTPUT3 In) : SV_Target
{
	float4 texColor = PhongModel(In.v3DPos, In.vNormal, g_DiffuseTex.Sample(SamplerStateWrap, In.vTexCoords*4));
	return texColor;
}


//-----------------------------------------------------------------------------
//GLOSS
//------------------------------------------------------------------------------
VS_OUTPUT4 VS_GLOSS(VS_INPUT In)
{	
	VS_OUTPUT4 result;

	float4 vFinalPos = mul(float4(In.vPosition, 1.0f), g_mWorld);
    float3 vFinalVec = mul(float4(In.vPosition, 0.0f), g_mWorld).xyz;
    result.v3DPos=vFinalPos;
    vFinalPos = mul(vFinalPos, g_mView);
    vFinalPos = mul(vFinalPos, g_mProj); 
    result.vNormal= normalize( mul( float4(In.vNormal, 0), g_mWorld ) ).xyz;
    result.vPosition = vFinalPos;
    result.vTexCoords=In.vTexCoords;
    float3 vpos=normalize(vFinalVec);
    float3 vTan=float3(vpos.z,0,-vpos.x);
   
	result.vB = normalize(cross(result.vNormal,vTan));    
    result.vT = normalize(cross(result.vNormal,result.vB));   
       
    return result;   
}
//----------------------------------------------------------------------------
float4 PS_GLOSS(VS_OUTPUT4 In) : SV_Target
{	
	float3x3 tnbMatrix = float3x3(In.vT, In.vB, In.vNormal);
	float3 Normal = normalize(g_NormalTex.Sample(SamplerStateWrap, In.vTexCoords*4).rgb * 2.0f - 1.0f);
	return PhongModelToon(In.v3DPos, Normal, g_DiffuseTex.Sample(SamplerStateWrap, In.vTexCoords*4), tnbMatrix);
}


//-----------------------------------------------------------------------------
//NORMAL
//------------------------------------------------------------------------------
VS_OUTPUT4 VS_NORMAL(VS_INPUT In)
{	
	VS_OUTPUT4 result;
    
    
    float4 vFinalPos = mul(float4(In.vPosition, 1.0f), g_mWorld);
    float3 vFinalVec = mul(float4(In.vPosition, 0.0f), g_mWorld).xyz;
    result.v3DPos=vFinalPos;
    vFinalPos = mul(vFinalPos, g_mView);
    vFinalPos = mul(vFinalPos, g_mProj); 
    result.vNormal= normalize( mul( float4(In.vNormal, 0), g_mWorld ) ).xyz;
    result.vPosition = vFinalPos;
    result.vTexCoords=In.vTexCoords;
    float3 vpos=normalize(vFinalVec);
    float3 vTan=float3(vpos.z,0,-vpos.x);
   
	result.vB = normalize(cross(result.vNormal,vTan));    
    result.vT = normalize(cross(result.vNormal,result.vB));   
       
    return result;  
}

//----------------------------------------------------------------------------
float4 PS_NORMAL(VS_OUTPUT4 In) : SV_Target
{
	float3x3 tnbMatrix = float3x3(In.vT, In.vB, In.vNormal);
	float3 Normal = normalize(g_NormalTex.Sample(SamplerStateWrap, In.vTexCoords*4).rgb * 2.0f - 1.0f);
	return PhongModelMap(In.v3DPos, Normal, g_DiffuseTex.Sample(SamplerStateWrap, In.vTexCoords*4), tnbMatrix);
}
//-----------------------------------------------------------------------------
//ALPHA
//------------------------------------------------------------------------------
VS_OUTPUT3 VS_ALPHA(VS_INPUT In)
{	
	VS_OUTPUT3 result;
    
    float4 vPos = float4(In.vPosition, 1.0f);
    float4 vFinalPos = mul(vPos, g_mWorld);
    result.v3DPos=vFinalPos;
    vFinalPos = mul(vFinalPos, g_mView);
    vFinalPos = mul(vFinalPos, g_mProj); 
    result.vNormal= normalize( mul( float4(In.vNormal, 0), g_mWorld ) ).xyz;
    result.vPosition = vFinalPos;
    result.vTexCoords=In.vTexCoords;
    return result;  
}
//----------------------------------------------------------------------------
float4 PS_ALPHA(VS_OUTPUT3 In) : SV_Target
{
	float4 Color =  PhongModel(In.v3DPos, In.vNormal, g_DiffuseTex.Sample(SamplerStateWrap, In.vTexCoords*4));
	Color[3] = 0.3;
	return Color;
}

//-----------------------------------------------------------------------------
//ALPHA FROM MATERIAL
//------------------------------------------------------------------------------
VS_OUTPUT3 VS_MATALPHA(VS_INPUT In)
{	
	VS_OUTPUT3 result;
    
    float4 vPos = float4(In.vPosition, 1.0f);
    float4 vFinalPos = mul(vPos, g_mWorld);
    result.v3DPos=vFinalPos;
    vFinalPos = mul(vFinalPos, g_mView);
    vFinalPos = mul(vFinalPos, g_mProj); 
    result.vNormal= normalize( mul( float4(In.vNormal, 0), g_mWorld ) ).xyz;
    result.vPosition = vFinalPos;
    result.vTexCoords=In.vTexCoords;
    return result;  
}
//----------------------------------------------------------------------------
float4 PS_MATALPHA(VS_OUTPUT3 In) : SV_Target
{
	float4 tColor = g_AlphaTex.Sample(SamplerStateWrap, In.vTexCoords*4);
	float4 Color = PhongModel(In.v3DPos, In.vNormal, tColor);
	Color[3] = tColor[3];
	return Color;
}


//----------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 RGBColor : SV_Target;  
};

VS_OUTPUT3 VS_GRASS_FANS(VS_INPUT In)
{	
    VS_OUTPUT3 output = (VS_OUTPUT3)0;
    output.vPosition  = float4(In.vPosition,1);
	output.vPosition  = mul(output.vPosition, g_mWorld);
	output.v3DPos	  = output.vPosition;
    output.vNormal    = mul(float4(In.vNormal,0), g_mWorld).xyz;
    output.vTexCoords = In.vTexCoords;    
    return output;
}

[maxvertexcount(21)]
void GS_GRASS_FANS(triangle VS_OUTPUT3 input[3], inout TriangleStream<VS_OUTPUT3> TriStream)
{	
    
    float3 faceEdgeA = input[1].vPosition.xyz - input[0].vPosition.xyz;
    float3 faceEdgeB = input[2].vPosition.xyz - input[0].vPosition.xyz;
	float3 faceEdgeC = input[1].vPosition.xyz - input[2].vPosition.xyz;
    //float3 faceNormal =cross(faceEdgeA, faceEdgeB);
    float3 faceNormal = faceEdgeC;
	float  Area=g_GrassHeight;
    
	//create the fins
	
	
	VS_OUTPUT3 vP0 = (VS_OUTPUT3)0;
	

	faceNormal = cross(faceEdgeB, input[2].vNormal);
	faceNormal = normalize( faceNormal );

	float3 eyeVectA = normalize(cross((input[0].vPosition.xyz - g_vEyePos),faceEdgeA));
	float3 eyeVectB = normalize(cross((input[2].vPosition.xyz - g_vEyePos),faceEdgeB));
	
	
	//V1
	vP0.vPosition = input[0].vPosition;
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[0].v3DPos;
    vP0.vTexCoords = float2(0,0);
	TriStream.Append( vP0 );

	//V2
	vP0.vPosition = input[0].vPosition+float4(eyeVectA*Area,0);
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[0].v3DPos;
    vP0.vTexCoords = float2(0,0.7);
	TriStream.Append( vP0 );

	//V3
	vP0.vPosition = input[2].vPosition;
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[2].v3DPos;
    vP0.vTexCoords = float2(1,0);
	TriStream.Append( vP0 );

	//V4
	vP0.vPosition = input[2].vPosition;
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[2].v3DPos;
    vP0.vTexCoords = float2(1,0);
	TriStream.Append( vP0 );

	//V5
	vP0.vPosition = input[2].vPosition+float4(eyeVectB*Area,0);
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[2].v3DPos;
    vP0.vTexCoords = float2(1,0.7);
	TriStream.Append( vP0 );

	//V6
	vP0.vPosition = input[0].vPosition+float4(eyeVectA*Area,0);
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[0].v3DPos;
    vP0.vTexCoords = float2(0,0.7);
	TriStream.Append( vP0 );

	/*
	faceNormal = cross(faceEdgeA, input[1].vNormal);
	faceNormal = normalize( faceNormal );

	//V7
	vP0.vPosition = input[0].vPosition;
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[0].v3DPos;
    vP0.vTexCoords = float2(0,0);
	TriStream.Append( vP0 );

	//V8
	vP0.vPosition = input[0].vPosition+float4(input[0].vNormal*Area,0);
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[0].v3DPos;
    vP0.vTexCoords = float2(0,0.7);
	TriStream.Append( vP0 );

	//V9
	vP0.vPosition = input[1].vPosition;
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[1].v3DPos;
    vP0.vTexCoords = float2(1,0);
	TriStream.Append( vP0 );

	//V10
	vP0.vPosition = input[1].vPosition;
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[1].v3DPos;
    vP0.vTexCoords = float2(1,0);
	TriStream.Append( vP0 );

	//V11
	vP0.vPosition = input[1].vPosition+float4(input[1].vNormal*Area,0);
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[1].v3DPos;
    vP0.vTexCoords = float2(1,0.7);
	TriStream.Append( vP0 );

	//V12
	vP0.vPosition = input[0].vPosition+float4(input[0].vNormal*Area,0);
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[0].v3DPos;
    vP0.vTexCoords = float2(0,0.7);
	TriStream.Append( vP0 );


	faceNormal = cross(faceEdgeC, input[2].vNormal);
	faceNormal = normalize( faceNormal );

	//V13
	vP0.vPosition = input[2].vPosition;
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[2].v3DPos;
    vP0.vTexCoords = float2(0,0);
	TriStream.Append( vP0 );

	//V14
	vP0.vPosition = input[2].vPosition+float4(input[2].vNormal*Area,0);
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[2].v3DPos;
    vP0.vTexCoords = float2(0,0.7);
	TriStream.Append( vP0 );

	//V15
	vP0.vPosition = input[1].vPosition;
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[1].v3DPos;
    vP0.vTexCoords = float2(1,0);
	TriStream.Append( vP0 );

	//V16
	vP0.vPosition = input[1].vPosition;
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[1].v3DPos;
    vP0.vTexCoords = float2(1,0);
	TriStream.Append( vP0 );

	//V17
	vP0.vPosition = input[1].vPosition+float4(input[1].vNormal*Area,0);
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[1].v3DPos;
    vP0.vTexCoords = float2(1,0.7);
	TriStream.Append( vP0 );

	//V18
	vP0.vPosition = input[2].vPosition+float4(input[2].vNormal*Area,0);
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[2].v3DPos;
    vP0.vTexCoords = float2(0,0.7);
	TriStream.Append( vP0 );
	*/

	TriStream.RestartStrip();
}

float4 PS_GRASS_FANS(VS_OUTPUT3 In) : SV_Target
{
	float2 uv = (float2(1,1) - In.vTexCoords);
	float4 texColor = g_GrassFin.Sample(SamplerStateWrap, uv);
	return texColor;
}

//---------------------------------------------------------------------------------------------------------
VS_OUTPUT3 VS_GRASS_SHELLS(VS_INPUT In)
{	
    VS_OUTPUT3 output = (VS_OUTPUT3)0;
    output.vPosition  = float4(In.vPosition,1);
	output.vPosition  = mul(output.vPosition, g_mWorld);
	output.v3DPos	  = output.vPosition;
    output.vNormal    = mul(float4(In.vNormal,0), g_mWorld).xyz;
    output.vTexCoords = In.vTexCoords;    
    return output;
}
//----------------------------------------------
[maxvertexcount(21)]
void GS_GRASS_SHELLS(triangle VS_OUTPUT3 input[3], inout TriangleStream<VS_OUTPUT3> TriStream)
{	
    
    float3 faceEdgeA = input[1].vPosition.xyz - input[0].vPosition.xyz;
    float3 faceEdgeB = input[2].vPosition.xyz - input[0].vPosition.xyz;
    float3 faceNormal =cross(faceEdgeA, faceEdgeB);
    float  Area=2*length(faceNormal);
    faceNormal = normalize( faceNormal );
    //Area=Area
    //Area=0.04;
    Area=g_GrassHeight*0.04;
	//create the fins
	
	
	 VS_OUTPUT3 vP0 = (VS_OUTPUT3)0;
	
	vP0.vPosition = input[0].vPosition+float4(input[0].vNormal*Area,0);
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[0].v3DPos;
    vP0.vTexCoords = float2(0,0);
	TriStream.Append( vP0 );
	vP0.vPosition = input[1].vPosition+float4(input[1].vNormal*Area,0);
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[1].v3DPos;
    vP0.vTexCoords = float2(1,0);
	TriStream.Append( vP0 );
	vP0.vPosition = input[2].vPosition+float4(input[2].vNormal*Area,0);
	vP0.vPosition = mul( vP0.vPosition, g_mView );
    vP0.vPosition = mul( vP0.vPosition, g_mProj );
	vP0.vNormal = faceNormal;
	vP0.v3DPos=input[1].v3DPos;
    vP0.vTexCoords = float2(1,1);
    TriStream.Append( vP0 );
	
	
	 TriStream.RestartStrip();
}

//----------------------------------------------------------------------------

float4 PS_GRASS_SHELLS(VS_OUTPUT3 In) : SV_Target
{
	float4 texColor = g_GrassShell.Sample(SamplerStateWrap, In.vTexCoords);
	//texColor[3] = 0.0;
	return texColor;
}


//--------------------------------------------------------------------------------------------
VS_OUTPUT3 VS_GRASS(VS_INPUT In)
{	
	VS_OUTPUT3 result;
    
    float4 vPos = float4(In.vPosition, 1.0f);
    float4 vFinalPos = mul(vPos, g_mWorld);
    result.v3DPos=vFinalPos;
    vFinalPos = mul(vFinalPos, g_mView);
    vFinalPos = mul(vFinalPos, g_mProj); 
    result.vNormal= normalize( mul( float4(In.vNormal, 0), g_mWorld ) ).xyz;
    result.vPosition = vFinalPos;
    result.vTexCoords=In.vTexCoords;
    return result;  
}
//----------------------------------------------------------------------------

float4 PS_GRASS(VS_OUTPUT3 In) : SV_Target
{
	float4 texColor = g_GrassSurface.Sample(SamplerStateWrap, In.vTexCoords*2);
	return texColor;
}

//--------------------------------------------------------------------------------------
// HAIR
//--------------------------------------------------------------------------------------

[maxvertexcount(21)]
void GS_HAIR(triangle VS_OUTPUT3 input[3], inout LineStream<VS_OUTPUT1> LinStream)
{	
    float3 faceEdgeA = input[1].vPosition.xyz - input[0].vPosition.xyz;
    float3 faceEdgeB = input[2].vPosition.xyz - input[0].vPosition.xyz;
	float3 faceEdgeC = input[2].vPosition.xyz - input[1].vPosition.xyz;
    float3 faceNormal =cross(faceEdgeA, faceEdgeB);
    faceNormal = normalize( faceNormal );
    float Area=g_GrassHeight;
	
	VS_OUTPUT1 vP0 = (VS_OUTPUT1)0;
	
	float step = 1.0/float(g_HairNumber);
	int i = 0;
	int j = 0;

	
	for (i=0; i< g_HairNumber; i++)
	{
		float u = float(i)*step;
		float m = (1-u)/2;
		float v = m;

		float4 hbase = input[0].vPosition*u+input[1].vPosition*m+input[2].vPosition*v;

		float4 vColor = PhongModel(hbase, input[1].vNormal, g_Diffuse);

		vP0.vPosition = hbase;
		vP0.vPosition = mul( vP0.vPosition, g_mView );
		vP0.vPosition = mul( vP0.vPosition, g_mProj );
		vP0.vColor = vColor*0.5;
		LinStream.Append(vP0);


		vP0.vPosition = hbase+float4(input[1].vNormal*g_GrassHeight,0);
		vP0.vPosition = mul( vP0.vPosition, g_mView );
		vP0.vPosition = mul( vP0.vPosition, g_mProj );
		vP0.vColor = vColor;
		LinStream.Append(vP0);

	}
	

	LinStream.RestartStrip();
}

float4 PS_HAIR(VS_OUTPUT1 In) : SV_Target
{
	float4 texColor = In.vColor;
	return texColor;
}

VS_OUTPUT3 VS_HAIR(VS_INPUT In)
{	
    VS_OUTPUT3 output = (VS_OUTPUT3)0;
    output.vPosition  = float4(In.vPosition,1);
	output.vPosition  = mul(output.vPosition, g_mWorld);
	output.v3DPos	  = output.vPosition;
    output.vNormal    = mul(float4(In.vNormal,0), g_mWorld).xyz;
    output.vTexCoords = In.vTexCoords;    
    return output;
}

//--------------------------------------------------------------------------------------
// REFLECT
//--------------------------------------------------------------------------------------
VS_OUTPUT2_REFLECT VS_REFLECT(VS_INPUT In)
{	
	VS_OUTPUT2_REFLECT result;
    
    float4 vPos = float4(In.vPosition, 1.0f);
    float4 vFinalPos = mul(vPos, g_mWorld);
    result.v3DPos=vFinalPos;
    vFinalPos = mul(vFinalPos, g_mView);
    vFinalPos = mul(vFinalPos, g_mProj); 
    result.vNormal= normalize( mul( float4(In.vNormal, 0), g_mWorld ) ).xyz;
    result.vPosition = vFinalPos;

	float3 incident = result.v3DPos - g_vEyePos;
	result.vReflect = reflect(incident, result.vNormal);

    return result;  
}
//----------------------------------------------------------------------------
float4 PS_REFLECT(VS_OUTPUT2_REFLECT In) : SV_Target
{
	return PhongModel(In.v3DPos, In.vNormal, g_CubeMapTex.Sample(SamplerStateWrap, In.vReflect));
}


//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------
technique10 RenderScene_Gouraud
{
    pass p0
    {   
        SetVertexShader( CompileShader( vs_4_0, VS_GOURAUD() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_GOURAUD() ) );
        SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepth, 1 );
    }
};
//---------------------------------------------------------------------------------------
technique10 RenderScene_phong
{
    pass p0
    {   
        SetVertexShader( CompileShader( vs_4_0, VS_PHONG() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_PHONG() ) );
        SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepth, 1 );
    }
};
//-------------------------------------------------------------------------------------
technique10 RenderScene_Texture_Diffuse
{
    pass p0
    {   
        SetVertexShader( CompileShader( vs_4_0, VS_DIFFUSE() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_DIFFUSE() ) );
        SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepth, 1 );
    }
};
//-------------------------------------------------------------------------------------
technique10 RenderScene_Texture_Gloss
{
    pass p0
    {   
        SetVertexShader( CompileShader( vs_4_0, VS_GLOSS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_GLOSS() ) );
        SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepth, 1 );
    }
};

//-------------------------------------------------------------------------------------
technique10 RenderScene_Texture_Normal
{
    pass p0
    {   
        SetVertexShader( CompileShader( vs_4_0, VS_NORMAL() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_NORMAL() ) );
        SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepth, 1 );
    }
};

//-------------------------------------------------------------------------------------
technique10 RenderScene_Texture_Alpha
{
    pass p0
    {   
        SetVertexShader( CompileShader( vs_4_0, VS_ALPHA() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ALPHA() ) );
       SetBlendState( AlphaOn, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
       
       
    }
};

//-------------------------------------------------------------------------------------
technique10 RenderScene_Grass_Fans
{
    pass p0
    {   
        SetVertexShader( CompileShader( vs_4_0, VS_GRASS_FANS() ) );
		SetGeometryShader( CompileShader( gs_4_0, GS_GRASS_FANS() ) );
        SetPixelShader( CompileShader( ps_4_0, PS_GRASS_FANS() ) );
       SetBlendState( AlphaOn, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
       SetDepthStencilState( EnableDepthTestingOnly, 0 );
       
    }
};

technique10 RenderScene_Grass_Shells
{
    pass p0
    {   
        SetVertexShader( CompileShader( vs_4_0, VS_GRASS_SHELLS() ) );
		SetGeometryShader( CompileShader( gs_4_0, GS_GRASS_SHELLS() ) );
        SetPixelShader( CompileShader( ps_4_0, PS_GRASS_SHELLS() ) );
       SetBlendState( AlphaOn, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
       
    }
};
technique10 RenderScene_Texture_Grass
{
    pass p0
    {   
        SetVertexShader( CompileShader( vs_4_0, VS_GRASS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_GRASS() ) );
       SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
       SetDepthStencilState( EnableDepth, 1 );
    }
};


//-------------------------------------------------------------------------------------
technique10 RenderScene_Mat_Alpha
{
    pass p0
    {   
        SetVertexShader( CompileShader( vs_4_0, VS_MATALPHA() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_MATALPHA() ) );
       SetBlendState( AlphaOn, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
       
       
    }
};

//---------------------------------------------------------------------------------------
technique10 RenderScene_reflection
{
    pass p0
    {   
        SetVertexShader( CompileShader( vs_4_0, VS_REFLECT() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_REFLECT() ) );
        SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepth, 1 );
    }
};

//---------------------------------------------------------------------------------------
technique10 RenderScene_hair
{
    pass p0
    {   
        SetVertexShader( CompileShader( vs_4_0, VS_HAIR() ) );
		SetGeometryShader( CompileShader( gs_4_0, GS_HAIR() ) );
        SetPixelShader( CompileShader( ps_4_0, PS_HAIR() ) );
        SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepth, 1 );
    }
};