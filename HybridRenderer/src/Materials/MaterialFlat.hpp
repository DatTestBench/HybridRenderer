#ifndef MATERIAL_FLAT_HPP
#define MATERIAL_FLAT_HPP
//Material with diffuse only

//Project includes
#include "Materials/Material.hpp"
#include "Materials/Texture.hpp"

class MaterialFlat final : public Material
{
public:
    MaterialFlat(ID3D11Device* pDevice,
                 const std::wstring& effectPath,
                 const std::string diffusePath,
                 const uint32_t id,
                 const bool hasTransparency = false)
        : Material(pDevice, effectPath, id, hasTransparency),
          m_pDiffuseMap(new Texture(pDevice, diffusePath))
    {
        m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
        if (!m_pDiffuseMapVariable->IsValid())
            std::wcout << L"Variable gDiffuseMap not found\n";

        m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
        if (!m_pMatWorldVariable->IsValid())
            std::wcout << L"Variable gWorldMatrix not found\n";
    }

    virtual ~MaterialFlat()
    {
        if (m_pMatWorldVariable)
            m_pMatWorldVariable->Release();

        if (m_pDiffuseMapVariable)
            m_pDiffuseMapVariable->Release();

        if (m_pDiffuseMap != nullptr)
            delete m_pDiffuseMap;
    }

    DEL_ROF(MaterialFlat)

    //Workers
    Elite::RGBColor Shade(const VertexOutput&, const Elite::FVector3&, const Elite::FVector3&, const Elite::FVector3&) const override
    {
        return {0.f, 0.f, 0.f};
    }

    //Setters
    /*D3D*/
    void SetMaps() override
    {
        if (m_pDiffuseMapVariable->IsValid())
            m_pDiffuseMapVariable->SetResource(m_pDiffuseMap->GetTextureView());
    }

    void SetMatrices(const Elite::FMatrix4& projectionMat, const Elite::FMatrix4& inverseViewMat /*This is the ONB*/, const Elite::FMatrix4& worldMat) override
    {
        auto worldViewProjection = projectionMat * Inverse(inverseViewMat) * worldMat;
        auto worldMatrix = worldMat;

        m_pMatWorldViewProjVariable->SetMatrix(&worldViewProjection(0, 0));
        m_pMatWorldVariable->SetMatrix(&worldMatrix(0, 0));
    }

    void SetScalars() override
    {
    }

private:
    /*General*/
    Texture* m_pDiffuseMap;
    /*D3D*/
    ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
    ID3DX11EffectMatrixVariable* m_pMatWorldVariable;
};

#endif // !MATERIAL_FLAT_HPP
