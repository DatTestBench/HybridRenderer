#ifndef MATERIAL_MANAGER_HPP
#define MATERIAL_MANAGER_HPP

//Standard Includes
#include <unordered_map>

// Project Includes
#include "Helpers/Singleton.hpp"
#include "Materials/Material.hpp"
#include "Geometry/Mesh.hpp"

enum SamplerType
{
    SPoint = 0,
    SLinear = 1,
    SAnisotropic = 2,
    SamplerSize = 3
};

class MaterialManager final : public Singleton<MaterialManager>
{
public:
    explicit MaterialManager(Token) : m_SamplerType(SPoint){};
    ~MaterialManager();
    DEL_ROF(MaterialManager)

    //External Item Manipulation
    void AddMaterial(Material* pMaterial);

    //Getters
    [[nodiscard]] constexpr auto GetMaterials() noexcept -> std::unordered_map<uint32_t, Material*>& { return m_Materials; }
    [[nodiscard]] auto GetMaterial(const int key) const noexcept -> Material* { return m_Materials.at(key); }
    [[nodiscard]] auto GetMaterial(Mesh* pObject) const noexcept -> Material* { return GetMaterial(pObject->GetMaterialId()); }
    [[nodiscard]] auto AmountOfMaterials() const noexcept -> uint32_t { return static_cast<uint32_t>(m_Materials.size()); }

    //Workers
    void ChangeFilterType();

private:

    //Data Members
    std::unordered_map<uint32_t, Material*> m_Materials;

    SamplerType m_SamplerType;
};

#endif // !MATERIAL_MANAGER_HPP
