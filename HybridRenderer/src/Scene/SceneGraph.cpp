#include "pch.h"
#include <iostream>
#include <string>
#include "Scene/SceneGraph.hpp"
#include "Rendering/Camera.hpp"

Camera* SceneGraph::m_pCamera = nullptr;

SceneGraph::~SceneGraph()
{
    for (auto pObject : m_Objects)
    {
        SafeDelete(pObject);
    }

    SafeDelete(m_pCamera);
}

#pragma region ExternalItemManipulation
void SceneGraph::AddObjectToGraph(Mesh* pObject, const int sceneIdx)
{
    m_Objects.push_back(pObject);
    m_pScenes.at(sceneIdx).push_back(pObject);
}

void SceneGraph::AddScene(const int sceneIdx)
{
    m_pScenes.try_emplace(sceneIdx);
}

void SceneGraph::SetCamera(const Elite::FPoint3& origin, const uint32_t windowWidth, const uint32_t windowHeight, const float fovD)
{
    if (m_pCamera == nullptr)
        m_pCamera = new Camera(origin, windowWidth, windowHeight, fovD);
}

void SceneGraph::ChangeCameraResolution(const uint32_t width, const uint32_t height)
{
    if (m_pCamera == nullptr)
        return;
    m_pCamera->SetResolution(width, height);
}
#pragma endregion

#pragma region Workers
void SceneGraph::Update(const float dT)
{
    float rotationSpeed;
    if (m_AreObjectsRotating)
    {
        rotationSpeed = 0.5f;
    }
    else
    {
        rotationSpeed = 0.f;
    }

    for (auto& i : GetCurrentSceneObjects())
    {
        i->Update(dT, rotationSpeed);
    }
}

void SceneGraph::IncreaseScene()
{
    m_CurrentScene = (m_CurrentScene == m_pScenes.size() - 1) ? 0 : m_CurrentScene + 1;
    std::cout << "Scene " << m_CurrentScene + 1 << " / " << m_pScenes.size() << std::endl;
}

void SceneGraph::DecreaseScene()
{
    m_CurrentScene = (m_CurrentScene == 0) ? static_cast<int32_t>(m_pScenes.size()) - 1 : m_CurrentScene - 1;
    std::cout << "Scene " << m_CurrentScene + 1 << " / " << m_pScenes.size() << std::endl;
}

void SceneGraph::ToggleRenderType()
{
    if (m_RenderSystem == D3D)
    {
        std::cout << "RenderType can not be changed in D3D mode\n";
        return;
    }

    m_RenderType = (m_RenderType == RenderTypeSize - 1) ? static_cast<RenderType>(0) : static_cast<RenderType>(m_RenderType + 1);

    std::cout << "Rendertype changed to ";
    switch (m_RenderType)
    {
    case RenderType::Color:
        std::cout << "show color\n";
        break;
    case RenderType::Depth:
        std::cout << "show depthbuffer\n";
        break;
    default:
        break;
    }
}

void SceneGraph::ToggleRenderSystem()
{
    m_RenderSystem = (m_RenderSystem == RenderSystem::RenderSystemSize - 1) ? static_cast<RenderSystem>(0) : static_cast<RenderSystem>(m_RenderSystem + 1);
    std::cout << "Rendersystem changed to ";
    switch (m_RenderSystem)
    {
    case Software:
        std::cout << "software\n";
        break;
    case D3D:
        std::cout << "D3D\n";
        break;
    default:
        break;
    }
    m_pCamera->ToggleRenderSystem(m_RenderSystem);
}

void SceneGraph::ToggleTransparency()
{
    if (m_RenderSystem == Software)
    {
        std::cout << "Transparency toggle not available in software mode\n";
        return;
    }

    m_ShowTransparency = !m_ShowTransparency;

    if (m_ShowTransparency)
    {
        std::cout << "Transparency on\n";
    }
    else
    {
        std::cout << "Transparency off\n";
    }
}

void SceneGraph::ToggleObjectRotation()
{
    m_AreObjectsRotating = !m_AreObjectsRotating;

    if (m_AreObjectsRotating)
    {
        std::cout << "Object rotation turned on\n";
    }
    else
    {
        std::cout << "Object rotation turned off\n";
    }
}
#pragma endregion
