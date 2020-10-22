#ifndef ELITE_MATERIAL_MANAGER
#define ELITE_MATERIAL_MANAGER

//Project includes
#include <unordered_map>
#include "Materials/Material.hpp"

enum SamplerType
{
    SPoint = 0,
    SLinear = 1,
    SAnisotropic = 2,
    SamplerSize = 3
};

class Mesh;

class MaterialManager
{
public:
    ~MaterialManager();
    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;
    MaterialManager(MaterialManager&&) = delete;
    MaterialManager& operator=(MaterialManager&&) = delete;

    //Singleton Functionality
    static MaterialManager* GetInstance();
    static void Destroy();

    //External Item Manipulation
    void AddMaterial(Material* pMaterial);

    //Getters
    std::unordered_map<int, Material*>* GetMaterials();
    Material* GetMaterial(int key) const;
    Material* GetMaterial(Mesh* pObject) const;
    size_t Size() const;

    //Workers
    void ChangeFilterType();

private:
    MaterialManager();

    //Data Members
    static MaterialManager* m_pMaterialManager;
    std::unordered_map<int, Material*> m_Materials;

    SamplerType m_SamplerType;
};

#endif // !ELITE_DX_MATERIAL_MANAGER
