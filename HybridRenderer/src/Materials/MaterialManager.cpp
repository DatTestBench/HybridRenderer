#include "pch.h"
#include "Materials/MaterialManager.hpp"
#include "Scene/SceneGraph.hpp"
#include "Geometry/Mesh.hpp"
MaterialManager* MaterialManager::m_pMaterialManager = nullptr;

MaterialManager::MaterialManager()
    : m_SamplerType{SPoint}
{
}

MaterialManager::~MaterialManager()
{
    for (std::pair<int, Material*> pMaterial : m_Materials)
    {
        delete pMaterial.second;
        pMaterial.second = nullptr;
    }
}

#pragma region SingletonFunctionality
MaterialManager* MaterialManager::GetInstance()
{
    if (m_pMaterialManager == nullptr)
        m_pMaterialManager = new MaterialManager();
    return m_pMaterialManager;
}

void MaterialManager::Destroy()
{
    delete GetInstance();
}
#pragma endregion

#pragma region ExternalItemManipulation
void MaterialManager::AddMaterial(Material* pMaterial)
{
    m_Materials.emplace(std::make_pair(pMaterial->GetId(), pMaterial));
}
#pragma endregion

#pragma region Getters
std::unordered_map<int, Material*>* MaterialManager::GetMaterials()
{
    return &m_Materials;
}

Material* MaterialManager::GetMaterial(int key) const
{
    return m_Materials.at(key);
}

Material* MaterialManager::GetMaterial(Mesh* pObject) const
{
    return GetMaterial(pObject->GetMaterialId());
}

size_t MaterialManager::Size() const
{
    return m_Materials.size();
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
