#ifndef ELITE_MATERIAL_MAPPED
#define ELITE_MATERIAL_MAPPED
//Material with all 4 maps, diffuse, normal, glossiness, specular

//Project includes
#include "Materials/Material.hpp"
#include "Materials/BRDF.hpp"
#include "Materials/Texture.hpp"

class MaterialMapped final : public Material
{
public:
    MaterialMapped(ID3D11Device* pDevice, const std::wstring& effectPath, const std::string& diffusePath, const std::string& normalPath, const std::string& glossPath, const std::string& specularPath,
                   const float shininess, const int id, bool hasTransparency = false)
        : Material{pDevice, effectPath, id, hasTransparency},
          m_pDiffuseMap{new Texture(pDevice, diffusePath)},
          m_pNormalMap{new Texture(pDevice, normalPath)},
          m_pGlossinessMap{new Texture(pDevice, glossPath)},
          m_pSpecularMap{new Texture(pDevice, specularPath)},
          m_Shininess{shininess}
    {
        m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
        if (!m_pDiffuseMapVariable->IsValid())
            std::wcout << L"Variable gDiffuseMap not found\n";

        m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
        if (!m_pNormalMapVariable->IsValid())
            std::wcout << L"Variable gNormalMap not found\n";

        m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
        if (!m_pGlossinessMapVariable->IsValid())
            std::wcout << L"Variable gGlossinessMap not found\n";

        m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
        if (!m_pSpecularMapVariable->IsValid())
            std::wcout << L"Variable gSpecularMap not found\n";

        m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
        if (!m_pMatWorldVariable->IsValid())
            std::wcout << L"Variable gWorldMatrix not found\n";

        m_pMatInverseViewVariable = m_pEffect->GetVariableByName("gInverseViewMatrix")->AsMatrix();
        if (!m_pMatInverseViewVariable->IsValid())
            std::wcout << L"Variable gInverseViewMatrix not found\n";

        m_pShininessVariable = m_pEffect->GetVariableByName("gShininess")->AsScalar();
        if (!m_pShininessVariable->IsValid())
            std::wcout << L"Variable gShininess not found\n";
    }

    virtual ~MaterialMapped()
    {
        //Releasing scalar variables
        if (m_pShininessVariable)
            m_pShininessVariable->Release();

        //Releasing matrix variables
        if (m_pMatInverseViewVariable)
            m_pMatInverseViewVariable->Release();
        if (m_pMatWorldVariable)
            m_pMatWorldVariable->Release();

        //Releasing map variables
        if (m_pSpecularMapVariable != nullptr)
            m_pSpecularMapVariable->Release();
        if (m_pGlossinessMapVariable != nullptr)
            m_pGlossinessMapVariable->Release();
        if (m_pNormalMapVariable != nullptr)
            m_pNormalMapVariable->Release();
        if (m_pDiffuseMapVariable != nullptr)
            m_pDiffuseMapVariable->Release();

        //Deleting software maps
        if (m_pDiffuseMap != nullptr)
            delete m_pDiffuseMap;
        if (m_pNormalMap != nullptr)
            delete m_pNormalMap;
        if (m_pGlossinessMap != nullptr)
            delete m_pGlossinessMap;
        if (m_pSpecularMap != nullptr)
            delete m_pSpecularMap;
    }

    MaterialMapped(const MaterialMapped&) = delete;
    MaterialMapped& operator=(const MaterialMapped&) = delete;
    MaterialMapped(MaterialMapped&&) = delete;
    MaterialMapped& operator=(MaterialMapped&&) = delete;

    //Workers
    /*Software*/
    Elite::RGBColor Shade(const VertexOutput& v, const Elite::FVector3& lightDirection, const Elite::FVector3& viewDirection, const Elite::FVector3& normal) const override
    {
        Elite::RGBColor tempColor = {0, 0, 0};

        if (m_pDiffuseMap != nullptr)
            tempColor += m_pDiffuseMap->Sample(v.uv);
        if (m_pSpecularMap != nullptr && m_pGlossinessMap != nullptr)
            tempColor += BRDF::Phong(m_pSpecularMap->Sample(v.uv), m_pGlossinessMap->SampleF(v.uv) * m_Shininess,
                                     lightDirection, viewDirection, normal);
        return tempColor;
    }

    //Setters
    /*D3D*/
    void SetMaps() override
    {
        if (m_pDiffuseMapVariable->IsValid())
            m_pDiffuseMapVariable->SetResource(m_pDiffuseMap->GetTextureView());
        if (m_pNormalMapVariable->IsValid())
            m_pNormalMapVariable->SetResource(m_pNormalMap->GetTextureView());
        if (m_pGlossinessMapVariable->IsValid())
            m_pGlossinessMapVariable->SetResource(m_pGlossinessMap->GetTextureView());
        if (m_pSpecularMapVariable->IsValid())
            m_pSpecularMapVariable->SetResource(m_pSpecularMap->GetTextureView());
    }

    void SetMatrices(const Elite::FMatrix4& projectionMat, const Elite::FMatrix4& inverseViewMat /*This is the OBN*/, const Elite::FMatrix4& worldMat) override
    {
        auto worldViewProjection = projectionMat * Elite::Inverse(inverseViewMat) * worldMat;
        auto worldMatrix = worldMat;
        auto inverseViewMatrix = inverseViewMat;

        m_pMatWorldViewProjVariable->SetMatrix(&worldViewProjection(0, 0));
        m_pMatWorldVariable->SetMatrix(&worldMatrix(0, 0));
        m_pMatInverseViewVariable->SetMatrix(&inverseViewMatrix(0, 0));
    }

    void SetScalars() override
    {
        m_pShininessVariable->SetFloat(m_Shininess);
    }

    //Getters
    /*D3D*/
    Elite::FVector3 GetMappedNormal(const VertexOutput& v) const override
    {
        if (m_pNormalMap == nullptr)
            return v.normal;

        const auto binormal = Cross(v.tangent, v.normal);
        auto tangentSpaceAxis = Elite::FMatrix3(v.tangent, binormal, v.normal);
        auto mappedNormal = m_pNormalMap->SampleV(v.uv);
        mappedNormal /= 255.f;
        mappedNormal = 2.f * mappedNormal - Elite::FVector3(1.f, 1.f, 1.f);
        mappedNormal = tangentSpaceAxis * mappedNormal;

        return mappedNormal;
    }

private:
    /*General*/
    float m_Shininess;
    Texture* m_pDiffuseMap;
    Texture* m_pNormalMap;
    Texture* m_pGlossinessMap;
    Texture* m_pSpecularMap;

    /*D3D*/
    ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
    ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
    ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;
    ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;

    ID3DX11EffectMatrixVariable* m_pMatWorldVariable;
    ID3DX11EffectMatrixVariable* m_pMatInverseViewVariable;
    ID3DX11EffectScalarVariable* m_pShininessVariable;
};
#endif // !ELITE_MATERIAL_MAPPED
