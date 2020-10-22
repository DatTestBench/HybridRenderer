#ifndef MESH_HPP
#define MESH_HPP

//DirectX Headers
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11effect.h>

//General includes
#include <vector>
#include <string>
//Project includes
#include "Helpers/EMath.h"
#include "Materials/Texture.hpp"
#include "Helpers/Vertex.hpp"
#include "Helpers/MeshParser.hpp"
#include "Materials/MaterialManager.hpp"


enum class PrimitiveTopology
{
    TriangleList,
    TriangleStrip
};

class Camera;

class Mesh
{
public:
    Mesh(ID3D11Device* pDevice, const std::string& modelPath, Material* pMaterial, const Elite::FPoint3& origin = {0, 0, 0});
    ~Mesh();
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) = delete;
    Mesh operator=(Mesh&&) = delete;

    //Workers
    /*General*/
    void Update(float dT, float rotationSpeed) noexcept;
    /*Software*/
    bool Rasterize(SDL_Surface* backBuffer, uint32_t* backBufferPixels, float* depthBuffer, uint32_t width,
                   uint32_t height);
    /*D3D*/
    void Render(ID3D11DeviceContext* pDeviceContext, Camera* pCamera) const noexcept;


    //Setters
    /*General*/
    void SetWorld(const Elite::FMatrix4& worldMat) { m_WorldMatrix = worldMat; }
    /*Software*/
    void SetScreenSpaceVertices(const std::vector<VertexOutput>& vertices) { m_SSVertices = vertices; }

    //Getters
    /*General*/
    int GetMaterialId() const { return m_MaterialId; }
    Elite::FMatrix4 GetWorld() const { return m_WorldMatrix; }
    const std::vector<VertexInput>& GetVertices() const { return m_VertexBuffer; }


private:
    /*General*/
    Elite::FMatrix4 m_WorldMatrix;
    Elite::FPoint3 m_Origin;
    float m_RotationAngle;
    int m_MaterialId;
    PrimitiveTopology m_Topology;

    /*Software*/
    std::vector<VertexInput> m_VertexBuffer;
    std::vector<uint32_t> m_IndexBuffer;
    std::vector<VertexOutput> m_SSVertices;

    bool AssembleTriangle(int idx, SDL_Surface* backBuffer, uint32_t* backBufferPixels, float* depthBuffer, uint32_t width, uint32_t height);
    std::tuple<bool, float, float, Elite::FVector3> IsPointInTriangle(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2, const Elite::FPoint2& pixelPoint) const noexcept;
    Elite::RGBColor PixelShading(const VertexOutput& v) const noexcept;
    std::tuple<Elite::FVector2, Elite::FVector2> MakeBoundingBox(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2, uint32_t maxScreenWidth = INT_MAX,
                                                                 uint32_t maxScreenHeight = INT_MAX) const noexcept;


    /*D3D*/
    ID3D11InputLayout* m_pVertexLayout;
    ID3D11Buffer* m_pVertexBuffer;
    ID3D11Buffer* m_pIndexBuffer;
    uint32_t m_AmountIndices;

    void MakeMesh(ID3D11Device* pDevice, const std::vector<uint32_t>& indices, const std::vector<VertexInput>& vertices);
};


#endif // !MESH_HPP