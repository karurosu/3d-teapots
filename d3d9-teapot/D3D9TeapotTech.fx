//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

///////////// UN-TWEAKABLES /////////////

float4x4 g_mWorld;                   // World matrix for object
float4x4 g_mView;					 // View matrix for object
float4x4 g_mWorldView;               // World View matrix for object
float4x4 g_mWorldViewProjection;     // World * View * Projection matrix
float3   g_vEyePos;					 // = float3(5,0,5);
//float    g_fTime;                    // App's time in seconds

///////////// TWEAKABLES /////////////

#define NUM_LIGHTS 3
float3  g_vLightPos[NUM_LIGHTS]   = {float3(0, 10, 0),
			 						 float3(10, 0, 0),
                            	     float3(-10, -10, 0)
                           			};

float4  g_vLightColor[NUM_LIGHTS] = {float4(0.6, 0.6, 0.6, 1),
							 	     float4(0, 0.2, 0.7, 1),
									 float4(1, 0.3, 0, 1)
                           			 };

float4 g_Ambient    = float4(0.1, 0.1, 0.1, 1);     // Material's ambient color
float4 g_Diffuse    = float4(0.7, 0.7, 0.7, 1);    // Material's diffuse color
float4 g_Specular   = float4(0.65, 0.65, 0.25, 1);
float  g_kDiffuse   = 1.0f;
float  g_PhongExp   = 128.0f;


//************ Textures *************/
texture g_DiffuseTex : DIFFUSE;
texture g_NormalTex  : NORMAL;
texture cubeMap		 : DIFFUSE;

sampler DiffuseTextureSampler = 
sampler_state
{
    Texture = <g_DiffuseTex>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler NormalTextureSampler = 
sampler_state
{
   Texture = <g_NormalTex>;
   MipFilter = LINEAR;
   MinFilter = LINEAR;
   MagFilter = LINEAR;
};

sampler ReflectTextureSampler = 
sampler_state
{
   Texture = <cubeMap>;
   MipFilter = LINEAR;
   MinFilter = LINEAR;
   MagFilter = LINEAR;
};

//--------------------------------------------------------------------------------------
// Vertex shader input structure
//--------------------------------------------------------------------------------------
struct VS_IN
{
    float3 vPos        : POSITION;	// vertex Pos from App
    float3 vNormal     : NORMAL;	// vertex Normal from App
    float2 vTexCoord   : TEXCOORD0;	// vertex Texture Coords from App
    float3 vTangent    : TEXCOORD1;	// vertex Tangent
    float3 vBinormal   : TEXCOORD2;	// vertex Binormal
};

//--------------------------------------------------------------------------------------
// Pixel shader input structure
//--------------------------------------------------------------------------------------
struct PS_IN
{
    float4 vPos        : POSITION;	// vertex Pos from App
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 vPos     : POSITION;  //  vertex position 
	float4 vPos3D   : TEXCOORD0;
	float4 vPosOrig   : TEXCOORD1;
	float2 vTexCoord   : TEXCOORD2;
	float3 vNormal   : NORMAL;
	float4 vColor   : COLOR0;
};

struct VS_OUTPUT_MAP
{
	float4 vPos     : POSITION;  //  vertex position 
	float4 vPos3D   : TEXCOORD0;
	float4 vPosOrig   : TEXCOORD1;
	float2 vTexCoord   : TEXCOORD2;
	float3 vNormal   : NORMAL0;
	float4 vColor   : COLOR0;
	
	float3 t: NORMAL1;
	float3 b: NORMAL2;
};

struct VS_OUTPUT_REFLECT
{
	float4 vPos     : POSITION;  //  vertex position 
	float4 vPos3D   : TEXCOORD0;
	float4 vPosOrig   : TEXCOORD1;
	float2 vTexCoord   : TEXCOORD2;
	float3 vNormal   : NORMAL;
	float4 vColor   : COLOR0;
	float3 vReflect : TEXCOORD3;
};

// Phong Model //
float4 PhongModel(float4 Pos, float3 Normal, float4 texColor)
{
	float4 Color = g_Ambient;
	float3 EyeToPos = normalize(Pos - g_vEyePos);
	float3 LightToPos = float3(0,0,0), Reflected = float3(0,0,0);
	float4 LightPos = float4(2,20,10, 1);
	float DiffuseIntensity  = 0, Spec = 0;
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		LightPos = float4(g_vLightPos[i], 1);
		LightToPos = normalize(Pos - LightPos);
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

//************ Shaders *************//
//************ Gouraud *************//

VS_OUTPUT VS_gouraud( VS_IN In )
{
	VS_OUTPUT Out;
    // Transform the position from object space to homogeneous projection space
	Out.vPos  = mul( float4(In.vPos, 1.0f), g_mWorldViewProjection  );
	Out.vPos3D = mul( float4(In.vPos, 1.0f), g_mWorldView  );
	Out.vPosOrig  = float4(In.vPos, 1.0f);
	Out.vNormal  = mul( float4(In.vNormal, 0.0f), g_mWorldView  ).xyz;
	Out.vTexCoord = In.vTexCoord;
	Out.vColor=PhongModel(Out.vPos3D,Out.vNormal, g_Diffuse);

	return Out;
}


float4 PS_gouraud( VS_OUTPUT In, uniform bool bTexture ) : COLOR
{
    return In.vColor;
}

//************ Phong *************//
VS_OUTPUT VS_phong( VS_IN In )
{
	VS_OUTPUT Out;
    // Transform the position from object space to homogeneous projection space
	Out.vPos  = mul( float4(In.vPos, 1.0f), g_mWorldViewProjection  );
	Out.vPos3D = mul( float4(In.vPos, 1.0f), g_mWorldView  );
	Out.vPosOrig  = float4(In.vPos, 1.0f);
	Out.vNormal  = mul( float4(In.vNormal, 0.0f), g_mWorldView  ).xyz;
	Out.vTexCoord = In.vTexCoord;
	Out.vColor = float4(0,0,0,1);

	return Out;
}


float4 PS_phong( VS_OUTPUT In, uniform bool bTexture ) : COLOR
{
    return PhongModel(In.vPos3D,In.vNormal, g_Diffuse);
}

//************ Texture *************//
VS_OUTPUT VS_text( VS_IN In )
{
	VS_OUTPUT Out;
    // Transform the position from object space to homogeneous projection space
	Out.vPos  = mul( float4(In.vPos, 1.0f), g_mWorldViewProjection  );
	Out.vPos3D = mul( float4(In.vPos, 1.0f), g_mWorldView  );
	Out.vPosOrig  = float4(In.vPos, 1.0f);
	Out.vNormal  = mul( float4(In.vNormal, 0.0f), g_mWorldView  ).xyz;
	Out.vTexCoord = In.vTexCoord;
	Out.vColor = float4(0,0,0,1);

	return Out;
}


float4 PS_text( VS_OUTPUT In, uniform bool bTexture , uniform float texScale) : COLOR
{
    return  PhongModel(In.vPos3D,In.vNormal, tex2D(DiffuseTextureSampler, In.vTexCoord*texScale));
}

//************ Alpha *************//
float4 PS_alpha( VS_OUTPUT In, uniform bool bTexture , uniform float texScale) : COLOR
{
	float4 Color = PhongModel(In.vPos3D,In.vNormal, tex2D(DiffuseTextureSampler, In.vTexCoord*texScale));
	Color[3] = 0.3;
    return  Color;
}


//************ Bump mapping *************//
VS_OUTPUT_MAP VS_map( VS_IN In )
{
	VS_OUTPUT_MAP Out;
    // Transform the position from object space to homogeneous projection space
	Out.vPos  = mul( float4(In.vPos, 1.0f), g_mWorldViewProjection  );
	Out.vPos3D = mul( float4(In.vPos, 1.0f), g_mWorldView  );
	Out.vPosOrig  = float4(In.vPos, 1.0f);
	Out.vNormal  = mul( float4(In.vNormal, 0.0f), g_mWorldView  ).xyz;
	Out.b = mul( float4(In.vBinormal, 0.0f), g_mWorldView  ).xyz;
	Out.t = mul( float4(In.vTangent, 0.0f), g_mWorldView  ).xyz;
	
	Out.vTexCoord = In.vTexCoord;
	Out.vColor = float4(0,0,0,1);

	return Out;
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

float4 PS_map( VS_OUTPUT_MAP In, uniform bool bTexture , uniform float texScale) : COLOR
{
	float3x3 tnbMatrix = float3x3(In.t, In.b, In.vNormal);
	float3 Normal = normalize(tex2D(NormalTextureSampler, In.vTexCoord*4.0).rgb * 2.0f - 1.0f);

    return  PhongModelMap(In.vPos3D, Normal, tex2D(DiffuseTextureSampler, In.vTexCoord*texScale), tnbMatrix);
}

//************ Toon mapping *************//
VS_OUTPUT_MAP VS_toon( VS_IN In )
{
	VS_OUTPUT_MAP Out;
    // Transform the position from object space to homogeneous projection space
	Out.vPos  = mul( float4(In.vPos, 1.0f), g_mWorldViewProjection  );
	Out.vPos3D = mul( float4(In.vPos, 1.0f), g_mWorldView  );
	Out.vPosOrig  = float4(In.vPos, 1.0f);
	Out.vNormal  = mul( float4(In.vNormal, 0.0f), g_mWorldView  ).xyz;
	Out.b = mul( float4(In.vBinormal, 0.0f), g_mWorldView  ).xyz;
	Out.t = mul( float4(In.vTangent, 0.0f), g_mWorldView  ).xyz;
	
	Out.vTexCoord = In.vTexCoord;
	Out.vColor = float4(0,0,0,1);

	return Out;
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

float4 PS_toon( VS_OUTPUT_MAP In, uniform bool bTexture , uniform float texScale) : COLOR
{
	float3x3 tnbMatrix = float3x3(In.t, In.b, In.vNormal);
	float3 Normal = normalize(tex2D(NormalTextureSampler, In.vTexCoord*4.0).rgb * 2.0f - 1.0f);

    return  PhongModelToon(In.vPos3D, Normal, float4(0,0.5,0.0,1), tnbMatrix);
}

//************ Reflection mapping *************//

VS_OUTPUT_REFLECT VS_reflect( VS_IN In )
{
	VS_OUTPUT_REFLECT Out;
    // Transform the position from object space to homogeneous projection space
	Out.vPos  = mul( float4(In.vPos, 1.0f), g_mWorldViewProjection  );
	Out.vPos3D = mul( float4(In.vPos, 1.0f), g_mWorldView  );
	Out.vPosOrig  = float4(In.vPos, 1.0f);
	Out.vNormal  = mul( float4(In.vNormal, 0.0f), g_mWorldView  ).xyz;
	Out.vTexCoord = In.vTexCoord;
	Out.vColor = float4(0,0,0,1);

	float3 incident = Out.vPos3D - g_vEyePos;
	Out.vReflect = reflect(incident, Out.vNormal);

	return Out;
}


float4 PS_reflect( VS_OUTPUT_REFLECT In, uniform bool bTexture ) : COLOR
{
    return PhongModel(In.vPos3D,In.vNormal, texCUBE(ReflectTextureSampler, In.vReflect));
}

//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------
technique RenderScene_Gouraud
{
    pass P0
    {   
        CullMode = NONE;
        VertexShader = compile vs_3_0 VS_gouraud();
        PixelShader  = compile ps_3_0 PS_gouraud(false); 
    }
}

technique RenderScene_Phong
{
    pass P0
    {   
        CullMode = NONE;
        VertexShader = compile vs_3_0 VS_phong();
        PixelShader  = compile ps_3_0 PS_phong(false); 
    }
}

technique RenderScene_Text
{
    pass P0
    {   
        CullMode = NONE;
        VertexShader = compile vs_3_0 VS_text();
        PixelShader  = compile ps_3_0 PS_text(false,1.0); 
    }
}

technique RenderScene_TextUV
{
    pass P0
    {   
        CullMode = NONE;
        VertexShader = compile vs_3_0 VS_text();
        PixelShader  = compile ps_3_0 PS_text(false,4.0); 
    }
}

technique RenderScene_Alpha
{
    pass P0
    {   
        CullMode = NONE;
        VertexShader = compile vs_3_0 VS_text();
        PixelShader  = compile ps_3_0 PS_alpha(false,1.0);
		AlphaBlendEnable = TRUE;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
    }
}

technique RenderScene_Bump
{
    pass P0
    {   
        CullMode = NONE;
        VertexShader = compile vs_3_0 VS_map();
        PixelShader  = compile ps_3_0 PS_map(false,4.0); 
    }
}

technique RenderScene_Toon
{
    pass P0
    {   
        CullMode = NONE;
        VertexShader = compile vs_3_0 VS_toon();
        PixelShader  = compile ps_3_0 PS_toon(false,4.0); 
    }
}

technique RenderScene_Reflect
{
    pass P0
    {   
        CullMode = NONE;
        VertexShader = compile vs_3_0 VS_reflect();
        PixelShader  = compile ps_3_0 PS_reflect(false); 
    }
}


