#include "pch.h"
#include "EMesh.h"
#include "EMaterial.h"
#include "ESceneGraph.h"
#include "ECamera.h"

Elite::Mesh::Mesh(ID3D11Device* pDevice, const std::string& modelPath, Material* pMaterial, const FPoint3& origin)
	: m_MaterialId{ pMaterial->GetId() }
	, m_IndexBuffer{  }
	, m_VertexBuffer{  }
	, m_pIndexBuffer{ nullptr }
	, m_pVertexLayout{ nullptr }
	, m_pVertexBuffer{ nullptr }
	, m_AmountIndices{  }
	, m_Origin{ origin }
	, m_Topology{ PrimitiveTopology::TriangleList } //Triangle strip is implemented, but can not be used currently
{
	MeshParser parser{};
	auto tuple = parser.ParseMesh(modelPath);

	MakeMesh(pDevice, std::get<1>(tuple), std::get<2>(tuple));
}

Elite::Mesh::~Mesh()
{
	if (m_pIndexBuffer)
		m_pIndexBuffer->Release();
	if (m_pVertexBuffer)
		m_pVertexBuffer->Release();
	if (m_pVertexLayout)
		m_pVertexLayout->Release();
}

#pragma region Workers
/*General*/
void Elite::Mesh::Update(float dT, float rotationSpeed)
{
	FMatrix3 rotationMatrix{};
	m_RotationAngle += rotationSpeed * dT;
	m_RotationAngle = (m_RotationAngle > (float)E_PI_2) ? m_RotationAngle - (float)E_PI_2 : m_RotationAngle;
	switch (SceneGraph::GetInstance()->GetRenderSystem())
	{
	case RenderSystem::Software:
		rotationMatrix = MakeRotationY(m_RotationAngle);
		break;
	case RenderSystem::D3D:
		//Flipping the axis the model is rotating around, because DirectX is lefthanded!
		rotationMatrix = MakeRotation(m_RotationAngle, FVector3(0, -1, 0));
		break;
	default:
		break;
	}
	m_WorldMatrix = MakeTranslation(FVector3(m_Origin)) * FMatrix4(rotationMatrix) * FMatrix4(MakeScale(1.f, 1.f, 1.f));
}

/*Software*/
bool Elite::Mesh::Rasterize(SDL_Surface* backBuffer, uint32_t* backBufferPixels, float* depthBuffer, uint32_t width, uint32_t height)
{
	bool meshHit = false;

	//Check if material on mesh should actually be rendered
	if (MaterialManager::GetInstance()->GetMaterial(m_MaterialId)->HasTransparancy())
	{
		return meshHit;
	}

	uint32_t increment{};
	switch (m_Topology)
	{
	case PrimitiveTopology::TriangleList:
		increment = 3;
		break;
	case PrimitiveTopology::TriangleStrip:
		increment = 1;
		break;
	default:
		break;
	}

	for (uint32_t i = 0; i < m_IndexBuffer.size() - 2; i += increment)
	{
		bool triHit = AssembleTriangle(i, backBuffer, backBufferPixels, depthBuffer, width, height);
		if (triHit)
		{
			meshHit = true;
		}
	}

	return meshHit;
}

bool Elite::Mesh::AssembleTriangle(int idx, SDL_Surface* backBuffer, uint32_t* backBufferPixels, float* depthBuffer, uint32_t width, uint32_t height)
{
	VertexOutput v0{};
	VertexOutput v1{};
	VertexOutput v2{};

	switch (m_Topology)
	{
	case Elite::PrimitiveTopology::TriangleList:
		v0 = m_SSVertices[m_IndexBuffer[uint64_t(idx)]];

		if (v0.pos.z < 0)
			return false;
		v1 = m_SSVertices[m_IndexBuffer[uint64_t(idx) + 1]];

		if (v1.pos.z < 0)
			return false;
		v2 = m_SSVertices[m_IndexBuffer[uint64_t(idx) + 2]];

		if (v2.pos.z < 0)
			return false;
		break;
	case Elite::PrimitiveTopology::TriangleStrip:
		if (m_IndexBuffer[idx] == m_IndexBuffer[uint64_t(idx) + 1] || m_IndexBuffer[uint64_t(idx) + 1] == m_IndexBuffer[uint64_t(idx) + 2] || m_IndexBuffer[uint64_t(idx) + 2] == m_IndexBuffer[idx])
			return false;
		v0 = m_SSVertices[m_IndexBuffer[idx]];
		if (v0.pos.z < 0)
			return false;
		v1 = m_SSVertices[m_IndexBuffer[uint64_t(idx) + 1 + uint64_t(idx % 2)]];
		if (v1.pos.z < 0)
			return false;
		v2 = m_SSVertices[m_IndexBuffer[uint64_t(idx) + 2 - uint64_t(idx % 2)]];
		if (v2.pos.z < 0)
			return false;
		break;
	default:
		break;
	}

	//Frustrum culling - Cut my ship into pieces...
	if (v0.culled || v1.culled || v2.culled)
	{
		return false;
	}

	float w0{};
	float w1{};
	float w2{};

	std::tuple<FVector2, FVector2> boundingBox = MakeBoundingBox(v0, v1, v2, width, height);
	FVector2 boundingBoxMin = std::get<0>(boundingBox);
	FVector2 boundingBoxMax = std::get<1>(boundingBox);

	RGBColor finalColor{};
	float depth = FLT_MAX;

	for (uint32_t r = uint32_t(boundingBoxMin.y); r < uint32_t(boundingBoxMax.y); ++r)
	{
		for (uint32_t c = uint32_t(boundingBoxMin.x); c < uint32_t(boundingBoxMax.x); ++c)
		{
			std::tuple<bool, float, float, FVector3> triResult = IsPointInTriangle(v0, v1, v2, FPoint2(float(c), float(r)));
			if (std::get<0>(triResult))
			{
				depth = std::get<1>(triResult);
				float linearInterpolatedDepth = std::get<2>(triResult);

				w0 = std::get<3>(triResult).x;
				w1 = std::get<3>(triResult).y;
				w2 = std::get<3>(triResult).z;

				if (depth < depthBuffer[c + (r * width)])
				{
					VertexOutput vInterpolated = Interpolate(v0, v1, v2, w0, w1, w2, linearInterpolatedDepth);

					switch (Elite::SceneGraph::GetInstance()->GetRenderType())
					{
					case RenderType::Color:
						finalColor = PixelShading(vInterpolated);
						break;
					case RenderType::Depth:
						finalColor = { Remap(depth, 0.985f, 1.f),Remap(depth, 0.985f, 1.f) ,Remap(depth, 0.985f, 1.f) };
						break;
					}

					depthBuffer[c + (r * width)] = depth;

					backBufferPixels[c + (r * width)] = SDL_MapRGB(backBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		}
	}
	return true;
}
std::tuple<bool, float, float, Elite::FVector3> Elite::Mesh::IsPointInTriangle(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2, const FPoint2& pixelPoint)
{
	//Is point in triangle
	FVector2 edgeA = v1.pos.xy - v0.pos.xy;
	FVector2 pointToSide = pixelPoint - v0.pos.xy;

	float signedAreaTriV2 = Cross(pointToSide, edgeA);
	if (signedAreaTriV2 < 0)
		return std::make_tuple(false, 0.f, 0.f, FVector3{});

	FVector2 edgeB = v2.pos.xy - v1.pos.xy;
	pointToSide = pixelPoint - v1.pos.xy;
	float signedAreaTriV0 = Cross(pointToSide, edgeB);
	if (signedAreaTriV0 < 0)
		return std::make_tuple(false, 0.f, 0.f, FVector3{});

	FVector2 edgeC = v0.pos.xy - v2.pos.xy;
	pointToSide = pixelPoint - v2.pos.xy;
	float signedAreaTriV1 = Cross(pointToSide, edgeC);
	if (signedAreaTriV1 < 0)
		return std::make_tuple(false, 0.f, 0.f, FVector3{});

	//Weight Calculations
	float signedAreaTriFull = Cross(FVector2(v1.pos.xy - v2.pos.xy), FVector2(v0.pos.xy - v1.pos.xy));
	if (signedAreaTriFull < 0)
		return std::make_tuple(false, 0.f, 0.f, FVector3{});

	float w0 = signedAreaTriV0 / signedAreaTriFull;
	float w1 = signedAreaTriV1 / signedAreaTriFull;
	float w2 = signedAreaTriV2 / signedAreaTriFull;

	float interpolatedDepth = 1.f / ((1.f / v0.pos.z) * w0 + (1.f / v1.pos.z) * w1 + (1.f / v2.pos.z) * w2);
	float linearInterpolatedDepth = 1.f / ((1.f / v0.pos.w) * w0 + (1.f / v1.pos.w) * w1 + (1.f / v2.pos.w) * w2);

	return std::make_tuple(true, interpolatedDepth, linearInterpolatedDepth, FVector3{ w0, w1, w2 });
}
Elite::RGBColor Elite::Mesh::PixelShading(const VertexOutput& v)
{
	RGBColor finalColor = { 0.f, 0.f, 0.f };
	FVector3 lightDirection = { 0.577f, -0.577f, 0.577f };
	float lightIntensity = 7.f;
	RGBColor lightColor = { 1.f, 1.f, 1.f };
	FVector3 mappedNormal = MaterialManager::GetInstance()->GetMaterial(m_MaterialId)->GetMappedNormal(v);

	float lambertCosine = Dot(-mappedNormal, lightDirection);

	if (lambertCosine < 0)
	{
		return finalColor;
	}

	finalColor += (lightColor * lightIntensity / (float)E_PI) * MaterialManager::GetInstance()->GetMaterial(m_MaterialId)->Shade(v, lightDirection, -v.viewDirection, mappedNormal) * lambertCosine;
	finalColor.MaxToOne();
	return finalColor;
}

std::tuple<Elite::FVector2, Elite::FVector2> Elite::Mesh::MakeBoundingBox(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2, uint32_t maxScreenWidth, uint32_t maxScreenHeight)
{
	FVector2 boundingBoxMin = { float(maxScreenWidth - 1), float(maxScreenHeight - 1) };
	FVector2 boundingBoxMax = { 0, 0 };

	for (auto& v : { v0, v1, v2 })
	{
		boundingBoxMin.x = std::min(boundingBoxMin.x, v.pos.x);
		boundingBoxMin.y = std::min(boundingBoxMin.y, v.pos.y);

		boundingBoxMax.x = std::max(boundingBoxMax.x, v.pos.x);
		boundingBoxMax.y = std::max(boundingBoxMax.y, v.pos.y);
	}

	boundingBoxMin.x = std::max(boundingBoxMin.x - 1, 0.f);
	boundingBoxMin.y = std::max(boundingBoxMin.y - 1, 0.f);

	boundingBoxMax.x = std::min(boundingBoxMax.x + 1, float(maxScreenWidth - 1));
	boundingBoxMax.y = std::min(boundingBoxMax.y + 1, float(maxScreenHeight - 1));

	return std::make_tuple(boundingBoxMin, boundingBoxMax);
}


/*D3D*/
void Elite::Mesh::Render(ID3D11DeviceContext* pDeviceContext, Camera* pCamera)
{
	//Check if material of mesh should actually be rendered
	if ((!SceneGraph::GetInstance()->IsTransparancyOn()) && MaterialManager::GetInstance()->GetMaterial(m_MaterialId)->HasTransparancy())
	{
		return;
	}

	//Set vertex buffer
	UINT stride = sizeof(VertexInput);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//Set index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Set the input layout
	pDeviceContext->IASetInputLayout(m_pVertexLayout);

	//Set primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Set Matrix
	MaterialManager::GetInstance()->GetMaterial(m_MaterialId)->SetMatrices(pCamera->GetProjectionMatrix(), pCamera->GetInverseViewMatrix(), m_WorldMatrix);

	//Set Maps (material-dependant)
	MaterialManager::GetInstance()->GetMaterial(m_MaterialId)->SetMaps();

	//Render a triangle
	D3DX11_TECHNIQUE_DESC techDesc;
	MaterialManager::GetInstance()->GetMaterial(m_MaterialId)->GetTechnique()->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		MaterialManager::GetInstance()->GetMaterial(m_MaterialId)->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);

		pDeviceContext->DrawIndexed(m_AmountIndices, 0, 0);
	}
}

void Elite::Mesh::MakeMesh(ID3D11Device* pDevice, const std::vector<uint32_t>& indices, const std::vector<VertexInput>& vertices)
{
	/*D3D Initialization*/
	//Create Vertex Layout
	HRESULT result = S_OK;
	static const uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "TEXCOORD";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 20;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


	//Create the input layout
	D3DX11_PASS_DESC passDesc;
	MaterialManager::GetInstance()->GetMaterial(m_MaterialId)->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);

	result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pVertexLayout);
	if (FAILED(result))
		return;

	//Create vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(VertexInput) * (uint32_t)vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = vertices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
		return;

	//Create index buffer
	m_AmountIndices = (uint32_t)indices.size();
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_AmountIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
		return;

	/*Software Initialization*/
	m_IndexBuffer = indices;
	for (auto vertex : vertices)
	{
		VertexInput v{};
		v.pos = { vertex.pos.x, vertex.pos.y, -vertex.pos.z };
		v.uv = vertex.uv;
		v.tangent = { vertex.tangent.x, vertex.tangent.y, -vertex.tangent.z };
		v.normal = { vertex.normal.x, vertex.normal.y, -vertex.normal.z };
		m_VertexBuffer.push_back(v);
	}
}

#pragma endregion