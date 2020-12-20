float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

float3 gLightDirection = {0.577f, -0.577f, 0.577f};
float4x4 gWorldMatrix : WORLD;
float4x4 gInverseViewMatrix : VIEWINVERSE;

float PI = 3.1415f;
float gLightIntensity = 7.0f;
float3 gLightColor = {1.f, 1.f, 1.f};
float gShininess = 25.0f;
int gSampleType = 0; // 0: Point; 1: Linear; 2: Anisotropic
int gRenderType = 0; // 0: full color; 1: specular; 2: normal(surface); 3: normal(mapped)

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
	float4 WorldPosition : POSITION;
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
	output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
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
	float cosineAngle = dot(reflection, viewDirection);

	if (cosineAngle > 0.f)
	{
		finalColor = float3(kS * pow(abs(cosineAngle), phongExponent));
	}

	return finalColor;
}

float3 MapNormal(VS_OUTPUT vertex)
{
	float3 binormal = cross(vertex.Normal, vertex.Tangent);
	float3x3 tangentSpaceAxis = float3x3(vertex.Tangent, binormal, vertex.Normal);
	float3 mappedNormal = SampleCustom(gNormalMap, vertex.UV);
	mappedNormal = 2.0f * mappedNormal - 1.f;
	mappedNormal = mul(mappedNormal, tangentSpaceAxis);
	return normalize(mappedNormal);
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    // Full color
    if (gRenderType == 0)
    {
        float3 viewDirection = normalize(input.WorldPosition.xyz - gInverseViewMatrix[3].xyz);
    
        // diffuse
	    float diffuseStrength = pow((dot(-MapNormal(input), gLightDirection) * 0.5f) + 0.5f, 2);
	    diffuseStrength = max(0.f, diffuseStrength);
	    diffuseStrength /= PI;
	    diffuseStrength *= gLightIntensity;
	    float3 diffuseColor = gLightColor * SampleCustom(gDiffuseMap, input.UV) * diffuseStrength;
	    
	    // phong
	    float3 specularValue = SampleCustom(gSpecularMap, input.UV);
	    float phongExponent = mul(SampleCustom(gGlossinessMap, input.UV).r, gShininess);
	    float3 specularColor = Phong(specularValue, phongExponent, -gLightDirection, viewDirection, MapNormal(input));
	    
	    return float4(diffuseColor + specularColor, 1.f);
	}
	
	// Specular
	if (gRenderType == 1)
	{
	    float3 viewDirection = normalize(input.WorldPosition.xyz - gInverseViewMatrix[3].xyz);
	    return float4(Phong(SampleCustom(gSpecularMap, input.UV), mul(SampleCustom(gGlossinessMap, input.UV).r, gShininess), -gLightDirection, viewDirection, MapNormal(input)), 1.f);
	}
	
	// Normal (surface)
	if (gRenderType == 2)
    {
        return saturate(float4(input.Normal, 1.f));
    }
    
    // Normal (mapped)
    if (gRenderType == 3)
    {
        return float4(MapNormal(input), 1.f);
    }
    
    return float4(0.f, 0.f, 0.f, 1.f);
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