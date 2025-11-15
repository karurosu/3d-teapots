//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer cbPerFrame : register( b0 )
{
	matrix g_mWorld;                   // World matrix for object
	matrix g_mView;					 // View matrix for object
	matrix g_mProj;					 // View matrix for object

	float3  g_vLightPos[3];
	float4  g_vLightColor[3];

	float4 g_Ambient;     // Material's ambient color
	float4 g_Diffuse;    // Material's diffuse color
	float4 g_Specular;
	float3 g_vEyePos;
	float  g_kDiffuse;
	float  g_PhongExp;
	
};
 
Texture2D Tile: register(t0);
SamplerState TileSampler;

//--PHONG models
float4 PhongModel(float4 Pos, float3 Normal, float4 texColor)
{
	float4 Color = g_Ambient;
	float3 EyeToPos = normalize(Pos.xyz - g_vEyePos);
	float3 LightToPos = float3(0,0,0), Reflected = float3(0,0,0);
	float4 LightPos = float4(2,20,10, 1);
	float DiffuseIntensity  = 0, Spec = 0;
	for (int i = 0; i < 3; i++)
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
	for (int i = 0; i < 3; i++)
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


//------------------------------------------------------------
struct VS_INPUT2
{
	float3 vPosition	:POSITION;
	//float4 vColor		:COLOR0;
};

struct VS_OUTPUT
{
	float3 vPosition	:WORLDPOS;
};
struct HS_OUTPUT
{
    float3 vPosition	: BEZIERPOS;
};
struct DS_OUTPUT
{
	float4 vPosition	: SV_POSITION;
	float3 v3DPos		: WORLDPOS;
	float2 uv			: TEXCOORD;
};
struct GS_OUTPUT
{
	float4 vPosition	: SV_POSITION;
	float4 vColor		:COLOR0;
	float3 v3DPos		:WORLDPOS;
	float3 vNormal		:NORMAL;
	float2 uv			:TEXCOORD;
};
struct PS_INPUT
{
	float4 vPosition	: SV_POSITION;
	float4 vColor		:COLOR0;
};


//--------------------------------------------------------------------------
//Vertex shader
//--------------------------------------------------------------------------
VS_OUTPUT SmoothVS(VS_INPUT2 In)
{	
	VS_OUTPUT result;
    result.vPosition=mul(float4(In.vPosition, 1.0f), g_mWorld);
    return result;  
}


//----------------------------------------------------------------------------------
//	Basic Hull Shader
//
//----------------------------------------------------------------------------------

struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[4]			: SV_TessFactor;
    float Inside[2]			: SV_InsideTessFactor;
};


HS_CONSTANT_DATA_OUTPUT MyConstantsHS( InputPatch<VS_OUTPUT, 16> ip,
                                                 uint PatchID : SV_PrimitiveID )
{	
    HS_CONSTANT_DATA_OUTPUT Output;
    
    float TessAmount = 64.0f;
    Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = TessAmount;
    Output.Inside[0] = Output.Inside[1] = TessAmount;
   
    return Output;
}
//----------------------------------------------------------------------------------
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("MyConstantsHS")]
HS_OUTPUT SmoothHS( InputPatch<VS_OUTPUT, 16> p, 
                                     uint i : SV_OutputControlPointID,
                                     uint PatchID : SV_PrimitiveID )
{
    
    
    HS_OUTPUT Output;
    Output.vPosition=p[i].vPosition;
    return Output;
}



//--------------------------------------------------------------------
//Domain shader
//--------------------------------------------------------------------
float3 beizer(float3 x1, float3 x2, float3 x3, float3 x4, float t) {
	static matrix<float,4,4> supermatrix = {  1.0f, -3.0f,  3.0f, -1.0f,
									   0.0f,  3.0f, -6.0f,  3.0f,
									   0.0f,  0.0f,  3.0f, -3.0f,
									   0.0f,  0.0f,  0.0f,  1.0f};

	float4x1 data = { 1.0f, t, pow(t,2), pow(t,3) };
	matrix<float,1,4> inputV;
	float3 Output;

	//X Component
	inputV[0][0] = x1.x;
	inputV[0][1] = x2.x;
	inputV[0][2] = x3.x;
	inputV[0][3] = x4.x;
	Output.x = mul(inputV, mul(supermatrix,data));

	//Y Component
	inputV[0][0] = x1.y;
	inputV[0][1] = x2.y;
	inputV[0][2] = x3.y;
	inputV[0][3] = x4.y;
	Output.y = mul(inputV, mul(supermatrix,data));

	//Z Component
	inputV[0][0] = x1.z;
	inputV[0][1] = x2.z;
	inputV[0][2] = x3.z;
	inputV[0][3] = x4.z;
	Output.z = mul(inputV, mul(supermatrix,data));

	return Output;
}

[domain("quad")]
DS_OUTPUT SmoothDS( HS_CONSTANT_DATA_OUTPUT input, 
                        float2 UV : SV_DomainLocation,
                        const OutputPatch<HS_OUTPUT, 16> bezpatch )
{
    DS_OUTPUT Output;
	
	//Calculate 4 points based on U:
	float3 X1 = beizer(bezpatch[0].vPosition,bezpatch[1].vPosition,bezpatch[2].vPosition,bezpatch[3].vPosition, UV.x);
	float3 X2 = beizer(bezpatch[4].vPosition,bezpatch[5].vPosition,bezpatch[6].vPosition,bezpatch[7].vPosition, UV.x);
	float3 X3 = beizer(bezpatch[8].vPosition,bezpatch[9].vPosition,bezpatch[10].vPosition,bezpatch[11].vPosition, UV.x);
	float3 X4 = beizer(bezpatch[12].vPosition,bezpatch[13].vPosition,bezpatch[14].vPosition,bezpatch[15].vPosition, UV.x);

	//Now calculate the final position based on the new points
	float3 WorldPos = beizer(X1,X2,X3,X4,UV.y);
	
	Output.vPosition =mul(float4(WorldPos,1.0f), g_mView);	
	Output.v3DPos =WorldPos;
	Output.uv = UV;
    return Output;    
}


//--------------------------------------------------------------------
//Geometry shader
//--------------------------------------------------------------------


//---------------------------------------------------------------------------

[maxvertexcount(3)]
void SmoothGS( triangle DS_OUTPUT input[3], inout TriangleStream<GS_OUTPUT> TriStream )
{	
    
    float3 faceEdgeA = input[1].v3DPos - input[0].v3DPos;
    float3 faceEdgeB = input[2].v3DPos - input[0].v3DPos;
    float3 faceNormal =cross(faceEdgeA, faceEdgeB);
    faceNormal = normalize( -faceNormal );
   
	GS_OUTPUT vP0 = (GS_OUTPUT)0;
    float4 color =  float4(0.0,1,0.0,1.0);

    vP0.vPosition = input[0].vPosition;
	vP0.v3DPos =input[0].v3DPos;
	vP0.vNormal=faceNormal;
	vP0.vColor = PhongModel(vP0.vPosition, vP0.vNormal, color);
	vP0.uv = input[0].uv;

    TriStream.Append( vP0 );
    
    vP0.vPosition = input[1].vPosition;
	vP0.v3DPos =input[1].v3DPos;
	vP0.vNormal=faceNormal;
    vP0.vColor = PhongModel(vP0.vPosition, vP0.vNormal, color);
	vP0.uv = input[1].uv;

	TriStream.Append( vP0 );
	
    vP0.vPosition = input[2].vPosition;
    vP0.v3DPos =input[2].v3DPos;
    vP0.vNormal=faceNormal;
	vP0.vColor = PhongModel(vP0.vPosition, vP0.vNormal, color);
	vP0.uv = input[2].uv;

	TriStream.Append( vP0 );
	TriStream.RestartStrip();
	
}

[maxvertexcount(3)]
void DispGS( triangle DS_OUTPUT input[3], inout TriangleStream<GS_OUTPUT> TriStream )
{	
    
    float3 faceEdgeA = input[1].v3DPos - input[0].v3DPos;
    float3 faceEdgeB = input[2].v3DPos - input[0].v3DPos;
    float3 faceNormal =cross(faceEdgeA, faceEdgeB);
	float4 d = 0;
    faceNormal = normalize( -faceNormal );
   
	GS_OUTPUT vP0 = (GS_OUTPUT)0;
    float4 color =  float4(0.0,1,0.0,1.0);

    vP0.vPosition = input[0].vPosition;
	vP0.v3DPos =input[0].v3DPos;
	vP0.vNormal=faceNormal;
	vP0.vColor = PhongModel(vP0.vPosition, vP0.vNormal, color);
	vP0.uv = input[0].uv;

	//Displacement
	d = Tile.SampleLevel( TileSampler, vP0.uv, 0 );
	vP0.vPosition = vP0.vPosition+float4(faceNormal*d.x*2,0);


    TriStream.Append( vP0 );
    
    vP0.vPosition = input[1].vPosition;
	vP0.v3DPos =input[1].v3DPos;
	vP0.vNormal=faceNormal;
    vP0.vColor = PhongModel(vP0.vPosition, vP0.vNormal, color);
	vP0.uv = input[1].uv;
	
	//Displacement
	d = Tile.SampleLevel( TileSampler, vP0.uv,0 );
	vP0.vPosition = vP0.vPosition+float4(faceNormal*d.x*2,0);

	TriStream.Append( vP0 );
	
    vP0.vPosition = input[2].vPosition;
    vP0.v3DPos =input[2].v3DPos;
    vP0.vNormal=faceNormal;
	vP0.vColor = PhongModel(vP0.vPosition, vP0.vNormal, color);
	vP0.uv = input[2].uv;
	
	//Displacement
	d = Tile.SampleLevel( TileSampler, vP0.uv,0 );
	vP0.vPosition = vP0.vPosition+float4(faceNormal*d.x*2,0);

	TriStream.Append( vP0 );
	TriStream.RestartStrip();
	
}

//--------------------------------------------------------------------
//Geometry shader
//--------------------------------------------------------------------

//---------------------------------------------
float4 SmoothPS(GS_OUTPUT In) : SV_TARGET
{
	return In.vColor;
}

float4 PhongPS(GS_OUTPUT In) : SV_TARGET
{
	return PhongModel(In.vPosition, -In.vNormal, float4(1.0,0.0,0.0,1.0));;
}

float4 TexturePS(GS_OUTPUT In) : SV_TARGET
{
	return PhongModel(In.vPosition, -In.vNormal, Tile.Sample( TileSampler, In.uv ));
}

float4 UVTexturePS(GS_OUTPUT In) : SV_TARGET
{
	return PhongModel(In.vPosition, -In.vNormal, Tile.Sample( TileSampler, In.uv*4 ));
}