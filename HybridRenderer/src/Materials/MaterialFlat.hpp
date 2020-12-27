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
                 const std::string& diffusePath,
                 const std::string_view name,
                 const bool hasTransparency = false)
        : Material(pDevice, effectPath, name, hasTransparency),
          m_pDiffuseMap(new Texture(pDevice, diffusePath))
    {

        D3DLOAD_VAR(m_pEffect, m_pDiffuseMapVariable, "gDiffuseMap", AsShaderResource)
        D3DLOAD_VAR(m_pEffect, m_pMatWorldVariable, "gWorldMatrix", AsMatrix)
    }

    virtual ~MaterialFlat()
    {
        SafeRelease(m_pMatWorldVariable);
        SafeRelease(m_pDiffuseMapVariable);

        SafeDelete(m_pDiffuseMap);
    }

    DEL_ROF(MaterialFlat)

    //Workers
    RGBColor Shade(const VertexOutput&, const glm::vec3&, const glm::vec3&, const glm::vec3&) const override { return RGBColor(0); }

    //Setters
    /*D3D*/
    void SetMaps() override
    {
        if (m_pDiffuseMapVariable->IsValid())
            m_pDiffuseMapVariable->SetResource(m_pDiffuseMap->GetTextureView());
    }

    void SetMatrices(const glm::mat4& projectionMat, const glm::mat4& viewMat, const glm::mat4& worldMat) override
    {
        auto worldViewProjection = projectionMat * viewMat * worldMat;
        auto worldMatrix = worldMat;

        m_pMatWorldViewProjVariable->SetMatrix(&worldViewProjection[0][0]);
        m_pMatWorldVariable->SetMatrix(&worldMatrix[0][0]);
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
