#include "pch.h"
#include <iostream>
#include <string>
#include "Scene/SceneGraph.hpp"

#include "Debugging/Logger.hpp"
#include "Helpers/magic_enum.hpp"
#include "Helpers/Timer.hpp"
#include "Rendering/Camera.hpp"
#include "Rendering/Renderer.hpp"

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

void SceneGraph::AddScene(const uint32_t sceneIdx)
{
    m_pScenes.try_emplace(sceneIdx);
}

void SceneGraph::SetCamera(const glm::vec3& origin, const uint32_t windowWidth, const uint32_t windowHeight, const float fovD)
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

void SceneGraph::RenderDebugUI() noexcept
{
    // ImGui Debug UI

    if(ImGui::Begin("Settings"))
    {
        // FPS Counter
        ImGui::Text((std::string("Framerate: ") + std::to_string(m_pTimer->GetFPS())).c_str());
        
        // Render System
        if (ImGui::BeginCombo("Render System", std::string(magic_enum::enum_name(m_RenderSystem)).c_str()))
        {
            for (auto [system, name] : magic_enum::enum_entries<RenderSystem>())
            {
                if (ImGui::Selectable(std::string(name).c_str()))
                {
                    m_RenderSystem = system;
                    m_ShouldUpdateRenderSystem = true;
                }
            }
            ImGui::EndCombo();
        }
        
        // Filter Modes

        // Transparency
        if (m_RenderSystem == D3D)
        {
            if (ImGui::Checkbox("Transparency", &m_ShowTransparency))
            {
                if (m_ShowTransparency)
                    LOG(LEVEL_INFO, "SceneGraph::RenderDebugUI()", "Transparancy On")
                else
                    LOG(LEVEL_INFO, "SceneGraph::RenderDebugUI()", "Transparancy Off")
            }
        }
        
        // Render Type

        if (m_RenderSystem == Software)
        {
            if (ImGui::BeginCombo("Render Type", std::string(magic_enum::enum_name(m_RenderType)).c_str()))
            {
                for (auto [type, name] : magic_enum::enum_entries<RenderType>())
                {
                    if (ImGui::Selectable(std::string(name).c_str()))
                    {
                        m_RenderType = type;
                        LOG(LEVEL_INFO, "SceneGraph::RenderDebugUI()", "Rendertype changed to " << magic_enum::enum_name(m_RenderType))
                    }
                }
                ImGui::EndCombo();
            }
        }
        
        // Scene Selection
        if (ImGui::BeginCombo("SceneSelection", std::to_string(m_CurrentScene).c_str()))
        {
            for (auto[id, objects] : m_pScenes)
            {
                if (ImGui::Selectable(std::to_string(id).c_str()))
                {
                    m_CurrentScene = id;
                    LOG(LEVEL_INFO, "SceneGraph::RenderDebugUI()", "Scene\n " << m_CurrentScene + 1 << " / " << m_pScenes.size())
                }
            }
            ImGui::EndCombo();
        }
        
        // Rotation
        if (ImGui::Checkbox("Rotation", &m_AreObjectsRotating))
        {
            if (m_AreObjectsRotating)
                LOG(LEVEL_INFO, "SceneGraph::RenderDebugUI()", "Object rotation turned On")
            else
                LOG(LEVEL_INFO, "SceneGraph::RenderDebugUI()", "Object rotation turned Off")
        }
        
    }
    ImGui::End();
}


#pragma endregion
