#ifndef MATERIAL_MAPPED_HPP
#define MATERIAL_MAPPED_HPP
//Material with all 4 maps, diffuse, normal, glossiness, specular

//Project includes
#include "Materials/Material.hpp"
#include "Materials/BRDF.hpp"
#include "Materials/Texture.hpp"
#include "Rendering/Camera.hpp"
#include <glm/gtc/matrix_access.hpp>
class MaterialMapped final : public Material
{
public:
    MaterialMapped(ID3D11Device* pDevice, const std::wstring& effectPath, const std::string& diffusePath,
                   const std::string& normalPath, const std::string& glossPath, const std::string& specularPath,
                   const float shininess, const uint32_t id, const bool hasTransparency = false)
        : Material(pDevice, effectPath, id, hasTransparency),
          m_pDiffuseMap(new Texture(pDevice, diffusePath)),
          m_pNormalMap(new Texture(pDevice, normalPath)),
          m_pGlossinessMap(new Texture(pDevice, glossPath)),
          m_pSpecularMap(new Texture(pDevice, specularPath)),
          m_Shininess(shininess)
    {
        m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
        if (!m_pDiffuseMapVariable->IsValid())
        {
            LOG(LEVEL_ERROR, "MaterialMapped::MaterialMapped()", "Variable gDiffuseMap not found")
            // LOG_OLD std::wcout << L"Variable gDiffuseMap not found\n";
        }          

        m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
        if (!m_pNormalMapVariable->IsValid())
        {
            LOG(LEVEL_ERROR, "MaterialMapped::MaterialMapped()", "Variable gNormalMap not found")
            // LOG_OLD std::wcout << L"Variable gNormalMap not found\n";
        }          

        m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
        if (!m_pGlossinessMapVariable->IsValid())
        {
            LOG(LEVEL_ERROR, "MaterialMapped::MaterialMapped()", "Variable gGlossinessMap not found")
            // LOG_OLD std::wcout << L"Variable gGlossinessMap not found\n";
        }            

        m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
        if (!m_pSpecularMapVariable->IsValid())
        {
            LOG(LEVEL_ERROR, "MaterialMapped::MaterialMapped()", "Variable gSpecularMap not found")
            // LOG_OLD std::wcout << L"Variable gSpecularMap not found\n";
        }          

        m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
        if (!m_pMatWorldVariable->IsValid())
        {
            LOG(LEVEL_ERROR, "MaterialMapped::MaterialMapped()", "Variable gWorldMatrix not found")
            // LOG_OLD std::wcout << L"Variable gWorldMatrix not found\n";
        }            

        m_pMatInverseViewVariable = m_pEffect->GetVariableByName("gInverseViewMatrix")->AsMatrix();
        if (!m_pMatInverseViewVariable->IsValid())
        {
            LOG(LEVEL_ERROR, "MaterialMapped::MaterialMapped()", "Variable gInverseViewMatrix not found")
            // LOG_OLD std::wcout << L"Variable gInverseViewMatrix not found\n";
        }

        m_pShininessVariable = m_pEffect->GetVariableByName("gShininess")->AsScalar();
        if (!m_pShininessVariable->IsValid())
        {
            LOG(LEVEL_ERROR, "MaterialMapped::MaterialMapped()", "Variable gShininess not found")
            // LOG_OLD std::wcout << L"Variable gShininess not found\n";
        }
            
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
        SafeDelete(m_pDiffuseMap);
        SafeDelete(m_pNormalMap);
        SafeDelete(m_pGlossinessMap);
        SafeDelete(m_pSpecularMap);
    }

    DEL_ROF(MaterialMapped)

    //Workers
    /*Software*/
    RGBColor Shade(const VertexOutput& v, const glm::vec3&, const glm::vec3&, const glm::vec3&) const override
    {
        const auto pCam = SceneGraph::GetCamera();
        const auto viewDirection = glm::normalize(v.worldPos - glm::vec3(glm::column(pCam->GetInverseViewMatrix(), 3)));
    
        RGBColor finalColor = {0.f, 0.f, 0.f};
        // variables
        const glm::vec3 lightDirection = {0.577f, -0.577f, -0.577f};

        const auto lightIntensity = 7.f;
        const RGBColor lightColor = {1.f, 1.f, 1.f};

        // normal
        const auto mappedNormal = GetMappedNormal(v);

        // diffuse
        RGBColor diffuseColor{0.f};
        if (m_pDiffuseMap!= nullptr)
        {
            float diffuseStrength = pow((glm::dot(-mappedNormal, lightDirection) * 0.5f) + 0.5f, 2.f);
            //auto diffuseStrength = glm::dot(-mappedNormal, lightDirection);
            diffuseStrength = std::max(0.f, diffuseStrength);
            diffuseStrength /= glm::pi<float>();
            diffuseStrength *= lightIntensity;
            diffuseColor = lightColor * m_pDiffuseMap->Sample(v.uv) * diffuseStrength;
        }

        // phong
        RGBColor specularColor{0.f};
        if (m_pSpecularMap != nullptr && m_pGlossinessMap != nullptr)
        {
            specularColor = BRDF::Phong(m_pSpecularMap->Sample(v.uv), m_pGlossinessMap->SampleF(v.uv) * m_Shininess, lightDirection, viewDirection, mappedNormal);
        }

        finalColor = diffuseColor + specularColor;

        MaxToOne(finalColor);

        return finalColor;


        
        //auto tempColor = RGBColor(0);
        //
        //if (m_pDiffuseMap != nullptr)
        //    tempColor += m_pDiffuseMap->Sample(v.uv);
        //if (m_pSpecularMap != nullptr && m_pGlossinessMap != nullptr)
        //    tempColor += BRDF::Phong(m_pSpecularMap->Sample(v.uv), m_pGlossinessMap->SampleF(v.uv) * m_Shininess,
        //                             lightDirection, viewDirection, normal);
        //return tempColor;
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

    void SetMatrices(const glm::mat4& projectionMat, const glm::mat4& inverseViewMat /*This is the OBN*/, const glm::mat4& worldMat) override
    {
        auto worldViewProjection = projectionMat * glm::inverse(inverseViewMat) * worldMat;
        auto worldMatrix = worldMat;
        auto inverseViewMatrix = inverseViewMat;

        m_pMatWorldViewProjVariable->SetMatrix(&worldViewProjection[0][0]);
        m_pMatWorldVariable->SetMatrix(&worldMatrix[0][0]);
        m_pMatInverseViewVariable->SetMatrix(&inverseViewMatrix[0][0]);
    }

    void SetScalars() override
    {
        m_pShininessVariable->SetFloat(m_Shininess);
    }

    //Getters
    /*Software*/
    glm::vec3 GetMappedNormal(const VertexOutput& v) const noexcept override
    {
        if (m_pNormalMap == nullptr)
            return v.normal;

        const auto binormal = glm::cross(v.tangent, v.normal);
        const auto tangentSpaceAxis = glm::mat3(v.tangent, binormal, v.normal);
        auto mappedNormal = m_pNormalMap->SampleV(v.uv);
        mappedNormal /= 255.f;
        mappedNormal = 2.f * mappedNormal - 1.f;
        mappedNormal = tangentSpaceAxis * mappedNormal;

        return glm::normalize(mappedNormal);
    }

private:
    /*General*/
    Texture* m_pDiffuseMap;
    Texture* m_pNormalMap;
    Texture* m_pGlossinessMap;
    Texture* m_pSpecularMap;
    float m_Shininess;

    /*D3D*/
    ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
    ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
    ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;
    ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;

    ID3DX11EffectMatrixVariable* m_pMatWorldVariable;
    ID3DX11EffectMatrixVariable* m_pMatInverseViewVariable;
    ID3DX11EffectScalarVariable* m_pShininessVariable;
};
#endif // !MATERIAL_MAPPED_HPP
