#include "pch.h"
#include "Materials/MaterialManager.hpp"
#include "Scene/SceneGraph.hpp"
#include "Geometry/Mesh.hpp"

MaterialManager::~MaterialManager()
{
    for (auto [id, pMat] : m_Materials)
    {
        SafeDelete(pMat);
    }
}

#pragma region ExternalItemManipulation
void MaterialManager::AddMaterial(Material* pMaterial)
{
    m_Materials.emplace(std::make_pair(pMaterial->GetId(), pMaterial));
}
#pragma endregion

#pragma region Workers

void MaterialManager::ChangeFilterType()
{
    if (SceneGraph::GetInstance()->GetRenderSystem() == Software)
    {
        std::cout << "FilterType can not be changed in software mode\n";
        return;
    }

    m_SamplerType = m_SamplerType == SamplerSize - 1 ? static_cast<SamplerType>(0) : static_cast<SamplerType>(m_SamplerType + 1);
    std::cout << "Sample type: ";
    switch (m_SamplerType)
    {
    case SPoint:
        std::cout << "Point\n";
        break;
    case SLinear:
        std::cout << "Linear\n";
        break;
    case SAnisotropic:
        std::cout << "Anisotropic\n";
        break;
    case SamplerSize:
        break;
    default:
        break;
    }

    for (auto& m : m_Materials)
    {
        m.second->GetSamplerType()->SetInt(m_SamplerType);
    }
}
#pragma endregion
