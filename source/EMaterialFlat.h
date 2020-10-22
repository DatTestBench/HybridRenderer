#ifndef ELITE_MATERIAL_FLAT
#define ELITE_MATERIAL_FLAT
//Material with diffuse only

//Project includes
#include "EMaterial.h"
#include "ETexture.h"

namespace Elite
{
	class MaterialFlat : public Material
	{
	public:
		MaterialFlat(ID3D11Device* pDevice, const std::wstring& effectPath, const std::string diffusePath, const int id, const bool hasTransparency = false)
			: Material{ pDevice, effectPath, id, hasTransparency}
			, m_pDiffuseMap{ new Texture(pDevice, diffusePath) }
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

		MaterialFlat(const MaterialFlat&) = delete;
		MaterialFlat& operator= (const MaterialFlat&) = delete;
		MaterialFlat(MaterialFlat&&) = delete;
		MaterialFlat& operator= (MaterialFlat&&) = delete;

		//Workers
		RGBColor Shade(const VertexOutput& v, const FVector3& lightDirection, const FVector3& viewDirection, const FVector3& normal) const override
		{
			return{0.f, 0.f, 0.f};
		}
		
		//Setters
		/*D3D*/
		void SetMaps() override
		{
			if (m_pDiffuseMapVariable->IsValid())
				m_pDiffuseMapVariable->SetResource(m_pDiffuseMap->GetTextureView());
		}

		void SetMatrices(const FMatrix4& projectionMat, const FMatrix4& inverseViewMat /*This is the ONB*/, const FMatrix4& worldMat) override
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
}
#endif // !ELITE_MATERIAL_FLAT
