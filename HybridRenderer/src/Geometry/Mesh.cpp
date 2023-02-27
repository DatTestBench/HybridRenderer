#include "pch.h"
#include "Geometry/Mesh.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Helpers/GeneralHelpers.hpp"
#include "Helpers/GeometryHelpers.hpp"
#include "Materials/Material.hpp"
#include "Materials/MaterialManager.hpp"
#include "Scene/SceneGraph.hpp"
#include "Rendering/Camera.hpp"


Mesh::Mesh(ID3D11Device* pDevice, const std::string& modelPath, Material* pMaterial, const glm::vec3& origin)
    : m_MaterialName(pMaterial->GetName()),
      m_Origin(origin),
      m_Topology(PrimitiveTopology::TriangleList) //Triangle strip is implemented, but can not be used currently
{
    MeshParser parser{};
    auto tuple = parser.ParseMesh(modelPath);

    MakeMesh(pDevice, std::get<1>(tuple), std::get<2>(tuple));
}

Mesh::~Mesh()
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
void Mesh::Update(const float dT, const float rotationSpeed) noexcept
{
    m_RotationAngle += rotationSpeed * dT;
    //rollover
    m_RotationAngle = m_RotationAngle > glm::two_pi<float>() ? m_RotationAngle - glm::two_pi<float>() : m_RotationAngle;
    
    auto yAxis = 1.f;
    // flip for DX
    if (SceneGraph::GetInstance()->GetRenderSystem() == D3D)
        yAxis = -1.f;

    m_WorldMatrix =
        glm::translate(m_Origin) *
        glm::rotate(m_RotationAngle, glm::vec3(0, yAxis, 0)) *
        glm::scale(glm::vec3(1));
}

/*Software*/
void Mesh::Rasterize(SDL_Surface* backBuffer, uint32_t* backBufferPixels, float* depthBuffer, const uint32_t width, const uint32_t height)
{
    //Check if material on mesh should actually be rendered
    if (MaterialManager::GetInstance()->GetMaterial(m_MaterialName)->HasTransparency())
        return;

    for (uint32_t i = 0; i < m_IndexBuffer.size() - 2; i += 3)
    {
        const auto i0 = m_IndexBuffer[i];
        const auto i1 = m_IndexBuffer[i+1];
        const auto i2 = m_IndexBuffer[i+2];

        const auto v0 = m_SSVertices.at(i0);
        const auto v1 = m_SSVertices.at(i1);
        const auto v2 = m_SSVertices.at(i2);

        
        if (v0.culled || v1.culled || v2.culled)
            continue;
        
        RasterizeTriangle(v0, v1, v2, backBuffer, backBufferPixels, depthBuffer, width, height);
    }
}


void Mesh::RasterizeTriangle(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2, SDL_Surface* backBuffer, uint32_t* backBufferPixels, float* depthBuffer, const uint32_t width, const uint32_t height) const noexcept 
{
    const auto boundingBox = MakeBoundingBox(v0, v1, v2, width, height);

    for (auto r = static_cast<uint32_t>(boundingBox.topLeft.y); r > static_cast<uint32_t>(boundingBox.bottomRight.y); --r)
    {
        for (auto c = static_cast<uint32_t>(boundingBox.topLeft.x); c < static_cast<uint32_t>(boundingBox.bottomRight.x); ++c)
        {
            TriangleResult triResult;
            bgh::CalculateWeightArea(glm::vec2(c, r), v0, v1, v2, triResult);

            if (!bgh::IsPointInTriangle(triResult))
                continue;

            const auto [w0, w1, w2] = triResult;

            // Depth test
            auto zDepth =
                ((1.f / v0.pos.z) * w0) +
                ((1.f / v1.pos.z) * w1) +
                ((1.f / v2.pos.z) * w2);

            zDepth = 1.f / zDepth;

            if (zDepth < depthBuffer[c + (r * width)])
            {
                depthBuffer[c + (r * width)] = zDepth;
                
                auto depth =     
                    ((1.f / v0.pos.w) * w0) +
                    ((1.f / v1.pos.w) * w1) +
                    ((1.f / v2.pos.w) * w2);
                
                depth = 1.f / depth;

                const auto interpolatedAttributes = Interpolate(v0, v1, v2, triResult, depth);

                RGBColor finalColor{};
                switch (SceneGraph::GetInstance()->GetSoftwareRenderType())
                {
                case SoftwareRenderType::Color:
                    finalColor = PixelShading(interpolatedAttributes);
                    break;
                case SoftwareRenderType::Depth:
                    finalColor = RGBColor(bme::Remap(zDepth, 0.985f, 1.f));
                    break;
                case SoftwareRenderType::Normal:
                    finalColor = glm::abs(interpolatedAttributes.normal);
                    break;
                case SoftwareRenderType::NormalMapped:
                    finalColor = glm::abs(MaterialManager::GetInstance()->GetMaterial(m_MaterialName)->GetMappedNormal(interpolatedAttributes));
                    break;
                }

                backBufferPixels[c + (r * width)] = SDL_MapRGB(backBuffer->format,
                                                               static_cast<uint8_t>(finalColor.r * 255),
                                                               static_cast<uint8_t>(finalColor.g * 255),
                                                               static_cast<uint8_t>(finalColor.b * 255));
            }
        }
    }
}

RGBColor Mesh::PixelShading(const VertexOutput& v) const noexcept
{
   //RGBColor finalColor = {0.f, 0.f, 0.f};

   const glm::vec3 lightDirection = {0.577f, -0.577f, -0.577f};

   //const auto lightIntensity = 7.f;
   //const RGBColor lightColor = {1.f, 1.f, 1.f};

   //const auto mappedNormal = MaterialManager::GetInstance()->GetMaterial(m_MaterialId)->GetMappedNormal(v);

   //const auto lambertCosine = std::max(0.f, glm::dot(-mappedNormal, lightDirection));

   //finalColor = (lightColor * lightIntensity / glm::pi<float>()) * MaterialManager::GetInstance()->GetMaterial(m_MaterialId)->Shade(v, lightDirection, v.viewDirection, mappedNormal) *
   //    lambertCosine;

   //MaxToOne(finalColor);
    const auto pMat = MaterialManager::GetInstance()->GetMaterial(m_MaterialName);
    return pMat->Shade(v, lightDirection, v.viewDirection, {});
}

BoundingBox2D Mesh::MakeBoundingBox(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2, const uint32_t maxScreenWidth, const uint32_t maxScreenHeight) const noexcept
{
    BoundingBox2D boundingBox{ {static_cast<float>(maxScreenWidth - 1), static_cast<float>(maxScreenHeight - 1)}, {0,0} };

    for (auto& v : {v0, v1, v2})
    {
        boundingBox.Expand(glm::vec2(v.pos));
    }
    boundingBox.AddMargin(1);
    
    boundingBox.Clamp({0.f, 0.f}, { maxScreenWidth, maxScreenHeight});
    return boundingBox;
}


/*D3D*/
void Mesh::Render(ID3D11DeviceContext* pDeviceContext, Camera* pCamera) const noexcept
{
    //Check if material of mesh should actually be rendered
    if (!SceneGraph::GetInstance()->IsTransparencyOn() && MaterialManager::GetInstance()->GetMaterial(m_MaterialName)->HasTransparency())
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
    MaterialManager::GetInstance()->GetMaterial(m_MaterialName)->SetMatrices(pCamera->GetProjectionMatrix(), pCamera->GetViewMatrix(), m_WorldMatrix);

    //Set Maps (material-dependant)
    MaterialManager::GetInstance()->GetMaterial(m_MaterialName)->SetMaps();

    //Render a triangle
    D3DX11_TECHNIQUE_DESC techDesc;
    MaterialManager::GetInstance()->GetMaterial(m_MaterialName)->GetTechnique()->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        MaterialManager::GetInstance()->GetMaterial(m_MaterialName)->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);

        pDeviceContext->DrawIndexed(m_AmountIndices, 0, 0);
    }
}

void Mesh::MakeMesh(ID3D11Device* pDevice, const std::vector<uint32_t>& indices, const std::vector<VertexInput>& vertices)
{
    /*D3D Initialization*/
    //Create Vertex Layout
    HRESULT result = S_OK;
    static const uint32_t numElements{4};
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
    MaterialManager::GetInstance()->GetMaterial(m_MaterialName)->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);

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
    bd.ByteWidth = sizeof(VertexInput) * static_cast<uint32_t>(vertices.size());
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;

    for (const auto& vertex : vertices)
    {
        VertexInput v{};
        v.pos = {vertex.pos.x, vertex.pos.y, -vertex.pos.z};
        v.uv = vertex.uv;
        v.tangent = {vertex.tangent.x, vertex.tangent.y, -vertex.tangent.z};
        v.normal = {vertex.normal.x, vertex.normal.y, -vertex.normal.z};
        m_HardwareVertexBuffer.push_back(v);
    }
    
    D3D11_SUBRESOURCE_DATA initData = {nullptr};
    initData.pSysMem = m_HardwareVertexBuffer.data();
    result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
    if (FAILED(result))
        return;

    //Create index buffer
    m_AmountIndices = static_cast<uint32_t>(indices.size());
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
    m_VertexBuffer = vertices;
}

#pragma endregion Workers
