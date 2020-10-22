float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

float3 gLightDirection = {0.577f, -0.577f, -0.577f};
float4x4 gWorldMatrix : WORLD;
float4x4 gInverseViewMatrix : VIEWINVERSE;

float PI = 3.1415f;
float gLightIntensity = 7.0f;
float3 gLightColor = {1.f, 1.f, 1.f};
float gShininess = 25.0f;
int gSampleType = 0; // Default sample type is PointSampling

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Border; // or Mirror or Clamp or Border
	AddressV = Clamp; // or Mirror or Clamp or Border
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Border; // or Mirror or Clamp or Border
	AddressV = Clamp; // or Mirror or Clamp or Border
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Border; // or Mirror or Clamp or Border
	AddressV = Clamp; // or Mirror or Clamp or Border
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};

RasterizerState gRasterizerState
{
	CullMode = back;
	FrontCounterClockwise = true;
};

BlendState gBlendState
{
	BlendEnable[0] = false;
	SrcBlend = one;
	DestBlend = zero;
	BlendOp = add;
	SrcBlendAlpha = one;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = all;
	DepthFunc = less;
	StencilEnable = false;

	StencilReadMask = 0x0F;
	StencilWriteMask = 0x0F;

	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;

	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;

	FrontFaceStencilPass = keep;
	BackFaceStencilPass = keep;

	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};



//--------------------------------------------------//
//	Input/Output Structs							//
//--------------------------------------------------//

struct VS_INPUT
{
	float3 Position : POSITION;
	float2 UV : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPostion : POSITION;
	float2 UV : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};


//--------------------------------------------------//
//	Helper Functions								//
//--------------------------------------------------//

float3 SampleCustom(Texture2D map, float2 uv)
{
	if (gSampleType == 0)
	{
		return map.Sample(samPoint, uv).xyz;
	}
	if (gSampleType == 1)
	{
		return map.Sample(samLinear, uv).xyz;
	}
	if (gSampleType == 2)
	{
		return map.Sample(samAnisotropic, uv).xyz;
	}
	return float3(0.f, 0.f, 0.f);
}

//--------------------------------------------------//
//	Vertex Shader									//
//--------------------------------------------------//

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.WorldPostion = mul(float4(input.Position, 1.f), gWorldMatrix);
	output.UV = input.UV;
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorldMatrix);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorldMatrix);

	return output;
}

//--------------------------------------------------//
//	Pixel Shader									//
//--------------------------------------------------//

float3 Phong(float3 kS, float phongExponent, float3 lightDirection, float3 viewDirection, float3 normal)
{	
	float3 finalColor = {0.f, 0.f, 0.f};
	float3 reflection = reflect(lightDirection, normal);
	float cosineAngle = (dot(reflection, viewDirection));

	if (cosineAngle > 0.0f)
	{
		finalColor = float3(kS * pow(cosineAngle, phongExponent));
	}

	return finalColor;
}

float3 MapNormal(VS_OUTPUT vertex)
{
	float3 binormal = cross(vertex.Normal, vertex.Tangent);
	float3x3 tangentSpaceAxis = transpose(float3x3(vertex.Tangent, binormal, vertex.Normal));
	float3 mappedNormal = SampleCustom(gNormalMap, vertex.UV);
	mappedNormal = normalize(2.0f * mappedNormal - 1.f);
	mappedNormal = mul(tangentSpaceAxis, mappedNormal);
	return normalize(mappedNormal);
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float lambertCosine = dot( -MapNormal(input), gLightDirection);
	float3 viewDirection = normalize(input.WorldPostion.xyz - gInverseViewMatrix[3].xyz);

	if (lambertCosine < 0.f)
	{
		lambertCosine = 0.0001f;
	}


	float3 tempColor = (gLightColor 
		* gLightIntensity / PI) 
		*(SampleCustom(gDiffuseMap, input.UV)
			+ Phong(SampleCustom(gSpecularMap, input.UV), mul(SampleCustom(gGlossinessMap, input.UV).r, gShininess), gLightDirection, viewDirection, MapNormal(input)) ) 
		* lambertCosine;

	return float4(saturate(tempColor),1.f);

}

//--------------------------------------------------//
//	Technique										//
//--------------------------------------------------//
technique11 DefaultTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS() ) );
	}
}