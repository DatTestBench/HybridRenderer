#ifndef MATERIAL_HPP
#define MATERIAL_HPP

//This is the "EFFECT". Holding data needed for the required DXEffect (the "SHADER")

#include "pch.h"

//General includes
#include <iostream>
#include <string>
#include <sstream>

//Project includes
#include "Helpers/Vertex.hpp"


class Material
{
public:
    Material(ID3D11Device* pDevice, const std::wstring& effectFile, const int id, const bool hasTransparency = false)
        : m_Id{id},
          m_HasTransparency{hasTransparency},
          m_pEffect{LoadEffect(pDevice, effectFile)}
    {
        m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
        if (!m_pTechnique->IsValid())
            std::wcout << L"Technique not valid\n";

        m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
        if (!m_pMatWorldViewProjVariable->IsValid())
            std::wcout << L"m_pMatWorldViewProjVariable not valid\n";

        m_pSamplerVariable = m_pEffect->GetVariableByName("gSampleType")->AsScalar();
        if (!m_pSamplerVariable->IsValid())
            std::wcout << L"m_pSamplerVariable not valid\n";
    }

    virtual ~Material()
    {
        if (m_pMatWorldViewProjVariable)
            m_pMatWorldViewProjVariable->Release();
        if (m_pTechnique)
            m_pTechnique->Release();
        if (m_pEffect)
            m_pEffect->Release();
    }

    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;
    Material(Material&&) = delete;
    Material& operator=(Material&&) = delete;

    //Workers
    /*Software*/
    virtual Elite::RGBColor Shade(const VertexOutput& v,
                                  const Elite::FVector3& lightDirection,
                                  const Elite::FVector3& viewDirection,
                                  const Elite::FVector3& normal) const = 0;

    //Setters
    /*D3D*/
    virtual void SetMaps() = 0;
    virtual void SetMatrices(const Elite::FMatrix4& projectionMat,
                             const Elite::FMatrix4& inverseViewMat /*This is the OBN*/,
                             const Elite::FMatrix4& worldMat) = 0;
    virtual void SetScalars() = 0;

    //Getters
    /*General*/
    int GetId() const { return m_Id; }
    bool HasTransparency() const { return m_HasTransparency; }

    /*Software*/
    virtual Elite::FVector3 GetMappedNormal(const VertexOutput& v) const { return v.normal; }

    /*D3D*/
    ID3DX11Effect* GetEffect() const { return m_pEffect; }
    ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; }
    ID3DX11EffectMatrixVariable* GetWorldViewProjMat() const { return m_pMatWorldViewProjVariable; }
    ID3DX11EffectScalarVariable* GetSamplerType() const { return m_pSamplerVariable; }

protected:
    /*General*/
    int m_Id;
    bool m_HasTransparency;
    /*D3D*/
    ID3DX11Effect* m_pEffect; // "SHADER"
    ID3DX11EffectTechnique* m_pTechnique;
    ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
    ID3DX11EffectScalarVariable* m_pSamplerVariable;

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

                std::wcout << ss.str() << std::endl;
            }
            else
            {
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
