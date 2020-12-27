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
    m_Materials.emplace(std::make_pair(pMaterial->GetName(), pMaterial));
}
#pragma endregion

#pragma region Workers

#pragma endregion
