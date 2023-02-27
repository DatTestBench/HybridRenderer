#ifndef MATERIAL_HPP
#define MATERIAL_HPP

//This is the "EFFECT". Holding data needed for the required DXEffect (the "SHADER")

//General includes
#include <iostream>
#include <string>
#include <sstream>

//Project includes
#include "Debugging/Logger.hpp"
#include "Helpers/Vertex.hpp"
#include "Scene/SceneGraph.hpp"
//#include "Scene/SceneGraph.hpp"

#define D3DLOAD_TECH(effect, tech, name) \
    tech = (effect)->GetTechniqueByName(name); \
    if (!(tech)->IsValid()) \
        LOG(LEVEL_ERROR, "Technique " << (name) << " not found")

#define D3DLOAD_VAR(effect, var, name, varType) \
    var = (effect)->GetVariableByName(name)->varType(); \
    if (!(var)->IsValid()) \
        LOG(LEVEL_ERROR, "Variable " << (name) << " not found")

class Material
{
public:
    Material(ID3D11Device* pDevice, const std::wstring& effectFile, const std::string_view name, const bool hasTransparency = false)
        : m_Name(name),
          m_HasTransparency(hasTransparency),
          m_pEffect(LoadEffect(pDevice, effectFile))
    {
        D3DLOAD_TECH(m_pEffect, m_pTechnique, "DefaultTechnique")
        D3DLOAD_VAR(m_pEffect, m_pMatWorldViewProjVariable, "gWorldViewProj", AsMatrix)
        D3DLOAD_VAR(m_pEffect, m_pSamplerVariable, "gSampleType", AsScalar)
        D3DLOAD_VAR(m_pEffect, m_pRenderTypeVariable, "gRenderType", AsScalar)
        
    }

    virtual ~Material()
    {
        //Releasing scalar variables
        SafeRelease(m_pSamplerVariable);
        SafeRelease(m_pRenderTypeVariable);

        //Releasing matrix variables
        SafeRelease(m_pMatWorldViewProjVariable);

        //Releasing technique and shader
        SafeRelease(m_pTechnique);
        SafeRelease(m_pEffect);
    }

    DEL_ROF(Material)

    //Workers
    /*Software*/
    virtual RGBColor Shade(const VertexOutput& v, const glm::vec3& lightDirection, const glm::vec3& viewDirection, const glm::vec3& normal) const = 0;

    //Setters
    /*D3D*/
    virtual void SetMaps() = 0;
    virtual void SetMatrices(const glm::mat4& projectionMat, const glm::mat4& viewMat, const glm::mat4& worldMat) = 0;
    virtual void SetScalars() = 0;

    void UpdateTypeSettings(const HardwareRenderType& renderType, const HardwareFilterType& samplerType) const noexcept
    {
        m_pSamplerVariable->SetInt(magic_enum::enum_integer(samplerType));
        m_pRenderTypeVariable->SetInt(magic_enum::enum_integer(renderType));
    }

    //Getters
    /*General*/
    [[nodiscard]] constexpr auto GetName() const noexcept -> std::string_view { return m_Name; }
    [[nodiscard]] constexpr auto HasTransparency() const noexcept -> bool { return m_HasTransparency; }

    /*Software*/
    [[nodiscard]] virtual auto GetMappedNormal(const VertexOutput& v) const noexcept -> glm::vec3 { return v.normal; }

    /*D3D*/

    [[nodiscard]] constexpr auto GetEffect() const noexcept -> ID3DX11Effect* { return m_pEffect; }
    [[nodiscard]] constexpr auto GetTechnique() const noexcept -> ID3DX11EffectTechnique* { return m_pTechnique; }
    [[nodiscard]] constexpr auto GetWorldViewProjMat() const noexcept -> ID3DX11EffectMatrixVariable* { return m_pMatWorldViewProjVariable; }
    [[nodiscard]] constexpr auto GetSamplerType() const noexcept -> ID3DX11EffectScalarVariable* { return m_pSamplerVariable; }
    [[nodiscard]] constexpr auto GetRenderType() const noexcept -> ID3DX11EffectScalarVariable* { return m_pRenderTypeVariable; }
protected:
    /*General*/
    std::string_view m_Name;
    bool m_HasTransparency;
    /*D3D*/
    ID3DX11Effect* m_pEffect; // "SHADER"
    ID3DX11EffectTechnique* m_pTechnique;
    ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
    ID3DX11EffectScalarVariable* m_pSamplerVariable;
    ID3DX11EffectScalarVariable* m_pRenderTypeVariable;

    //Handles "SHADER" compilation
    static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& effectFile)
    {
        HRESULT result = S_OK;
        ID3D10Blob* pErrorBlob = nullptr;
        ID3DX11Effect* pEffect;

        DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined ( _DEBUG )
			shaderFlags |= D3DCOMPILE_DEBUG;
			shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        result = D3DX11CompileEffectFromFile(effectFile.c_str(),
                                             nullptr,
                                             nullptr,
                                             shaderFlags,
                                             0,
                                             pDevice,
                                             &pEffect,
                                             &pErrorBlob);

        if (FAILED(result))
        {
            if (pErrorBlob != nullptr)
            {
                auto* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

                std::wstringstream ss;
                for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
                    ss << pErrors[i];

                OutputDebugStringW(ss.str().c_str());
                pErrorBlob->Release();
                pErrorBlob = nullptr;

                // todo add compat for wstring in logger (probably just converting from string to wstring internally
                // LOG(LEVEL_ERROR, ss)
                std::wcout << ss.str() << std::endl;
            }
            else
            {
                //LOG(LEVEL_ERROR, "EffectLoader: Failed to CreateEventFromFile!")
                std::wstringstream ss;
                ss << "EffectLoader: Failed to CreateEventFromFile!\nPath: " << effectFile;
                std::wcout << ss.str() << std::endl;
                return nullptr;
            }
        }
        return pEffect;
    }
};

#endif // !MATERIAL_HPP
