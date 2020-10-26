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
        {
            LOG(LEVEL_ERROR, "MaterialFlat::MaterialFlat()", "Variable gDiffuseMap not found")
            // LOG_OLD std::wcout << L"Variable gDiffuseMap not found\n";
        }
        
        m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
        if (!m_pMatWorldVariable->IsValid())
        {
            LOG(LEVEL_ERROR, "MaterialFlat::MaterialFlat()", "Variable gWorldMatrix not found")
            // LOG_OLD std::wcout << L"Variable gWorldMatrix not found\n";
        }
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
    RGBColor Shade(const VertexOutput&, const glm::vec3&, const glm::vec3&, const glm::vec3&) const override { return RGBColor(0); }

    //Setters
    /*D3D*/
    void SetMaps() override
    {
        if (m_pDiffuseMapVariable->IsValid())
            m_pDiffuseMapVariable->SetResource(m_pDiffuseMap->GetTextureView());
    }

    void SetMatrices(const glm::mat4& projectionMat, const glm::mat4& inverseViewMat /*This is the ONB*/, const glm::mat4& worldMat) override
    {
        auto worldViewProjection = projectionMat * glm::inverse(inverseViewMat) * worldMat;
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
