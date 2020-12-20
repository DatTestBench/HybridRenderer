float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;

float3 gLightDirection = {0.577f, -0.577f, 0.577f};
float4x4 gWorldMatrix : WORLD;

float PI = 3.1415f;
float gLightIntensity = 7.0f;
float3 gLightColor = {1.f, 1.f, 1.f};
int gSampleType = 0; // Default sample type is PointSampling
int gRenderType = 0;

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
	CullMode = none;
	FrontCounterClockwise = false;
};

BlendState gBlendState
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = zero;
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
	float4 WorldPosition : COLOR;
	float2 UV : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

//--------------------------------------------------//
//	Helper Functions								//
//--------------------------------------------------//

float4 SampleCustom(Texture2D map, float2 uv)
{
	if (gSampleType == 0)
	{
		return map.Sample(samPoint, uv);
	}
	if (gSampleType == 1)
	{
		return map.Sample(samLinear, uv);
	}
	if (gSampleType == 2)
	{
		return map.Sample(samAnisotropic, uv);
	}
	return float4(0.f, 0.f, 0.f, 0.f);
}


//--------------------------------------------------//
//	Vertex Shader									//
//--------------------------------------------------//

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
	output.UV = input.UV;
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorldMatrix);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorldMatrix);

	return output;
}

//--------------------------------------------------//
//	Pixel Shader									//
//--------------------------------------------------//

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float lambertCosine = dot(-input.Normal, gLightDirection);
	float4 color = SampleCustom(gDiffuseMap, input.UV);
	float3 tempColor = (gLightColor 
		* gLightIntensity / PI) 
		* color.xyz
		* lambertCosine;

	return saturate(color);
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
