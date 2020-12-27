#include "pch.h"
#include <iostream>
#include <string>
#include "Scene/SceneGraph.hpp"

#include "Debugging/Logger.hpp"
#include "Geometry/Mesh.hpp"
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
        rotationSpeed = 0.1f;
    }
    else
    {
        rotationSpeed = 0.f;
    }

    for (auto pMesh : GetCurrentSceneObjects())
    {
        pMesh->Update(dT, rotationSpeed);
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
        if (ImGui::BeginCombo("Render System",  ENUM_TO_C_STR(m_RenderSystem)))
        {
            for (auto [system, name] : magic_enum::enum_entries<RenderSystem>())
            {
                if (ImGui::Selectable(C_STR_FROM_VIEW(name)) && system != m_RenderSystem)
                {
                    m_RenderSystem = system;
                    m_ShouldUpdateRenderSystem = true;

                    LOG(LEVEL_INFO, "Rendersystem changed to " << magic_enum::enum_name(m_RenderSystem))
                }
            }
            ImGui::EndCombo();
        }

        // Transparency
        if (m_RenderSystem == D3D)
        {
            if (ImGui::Checkbox("Transparency", &m_ShowTransparency))
            {
                if (m_ShowTransparency)
                    LOG(LEVEL_INFO, "Transparancy On")
                else
                    LOG(LEVEL_INFO, "Transparancy Off")
            }
        }
        
        // Software Render Type

        if (m_RenderSystem == Software)
        {
            if (ImGui::BeginCombo("Render Type", ENUM_TO_C_STR(m_SoftwareRenderType)))
            {
                for (auto [type, name] : magic_enum::enum_entries<SoftwareRenderType>())
                {
                    if (ImGui::Selectable(C_STR_FROM_VIEW(name)) && type != m_SoftwareRenderType)
                    {
                        m_SoftwareRenderType = type;
                        LOG(LEVEL_INFO, "Rendertype changed to " << magic_enum::enum_name(m_SoftwareRenderType))
                    }
                }
                ImGui::EndCombo();
            }
        }

        // Hardware Render Type and Filtering
        if (m_RenderSystem == D3D)
        {
            if (ImGui::BeginCombo("Render Type", ENUM_TO_C_STR(m_HardwareRenderType)))
            {
                for (auto [type, name] : magic_enum::enum_entries<HardwareRenderType>())
                {
                    if (ImGui::Selectable(C_STR_FROM_VIEW(name)) && type != m_HardwareRenderType)
                    {
                        m_HardwareRenderType = type;
                        m_ShouldUpdateHardwareTypes = true;
                        LOG(LEVEL_INFO, "Rendertype changed to " << magic_enum::enum_name(m_HardwareRenderType))
                    }
                }
                ImGui::EndCombo();
            }

            if (ImGui::BeginCombo("Filter Type", ENUM_TO_C_STR(m_HardwareFilterType)))
            {
                for (const auto [type, name] : magic_enum::enum_entries<HardwareFilterType>())
                {
                    if (ImGui::Selectable(C_STR_FROM_VIEW(name)) && type != m_HardwareFilterType)
                    {
                        m_HardwareFilterType = type;
                        m_ShouldUpdateHardwareTypes = true;
                        LOG(LEVEL_INFO, "Filtertype changed to " << magic_enum::enum_name(m_HardwareFilterType))
                    }
                }
                ImGui::EndCombo();
            }
        }
        
        // Scene Selection
        if (ImGui::BeginCombo("SceneSelection", TO_C_STR(m_CurrentScene)))
        {
            for (auto[id, objects] : m_pScenes)
            {
                if (ImGui::Selectable(TO_C_STR(id)) && m_CurrentScene != id)
                {
                    m_CurrentScene = id;
                    LOG(LEVEL_INFO, "Scene\n " << m_CurrentScene + 1 << " / " << m_pScenes.size())
                }
            }
            ImGui::EndCombo();
        }
        
        // Rotation
        if (ImGui::Checkbox("Rotation", &m_AreObjectsRotating))
        {
            if (m_AreObjectsRotating)
                LOG(LEVEL_INFO, "Object rotation turned On")
            else
                LOG(LEVEL_INFO, "Object rotation turned Off")
        }

        // Camera Variables
        const auto pCam = GetCamera();
        
        ImGui::Text("Position");
        const auto pos = pCam->GetPosition();
        ImGui::BulletText("x: "); ImGui::SameLine(); ImGui::Text(TO_C_STR(pos.x));
        ImGui::BulletText("y: "); ImGui::SameLine(); ImGui::Text(TO_C_STR(pos.y));
        ImGui::BulletText("z: "); ImGui::SameLine(); ImGui::Text(TO_C_STR(pos.z));

        ImGui::Text("Rotation");
        const auto pitch = pCam->GetPitch();
        const auto yaw = pCam->GetYaw();
        ImGui::BulletText("pitch: "); ImGui::SameLine(); ImGui::Text(TO_C_STR(pitch));
        ImGui::BulletText("yaw: "); ImGui::SameLine(); ImGui::Text(TO_C_STR(yaw));

        ImGui::Text("Forward");
        const auto forward = pCam->GetForward();
        ImGui::BulletText("x: "); ImGui::SameLine(); ImGui::Text(TO_C_STR(forward.x));
        ImGui::BulletText("y: "); ImGui::SameLine(); ImGui::Text(TO_C_STR(forward.y));
        ImGui::BulletText("z: "); ImGui::SameLine(); ImGui::Text(TO_C_STR(forward.z));

        ImGui::Text("Right");
        const auto right = pCam->GetForward();
        ImGui::BulletText("x: "); ImGui::SameLine(); ImGui::Text(TO_C_STR(right.x));
        ImGui::BulletText("y: "); ImGui::SameLine(); ImGui::Text(TO_C_STR(right.y));
        ImGui::BulletText("z: "); ImGui::SameLine(); ImGui::Text(TO_C_STR(right.z));

        ImGui::Text("Up");
        const auto up = pCam->GetForward();
        ImGui::BulletText("x: "); ImGui::SameLine(); ImGui::Text(TO_C_STR(up.x));
        ImGui::BulletText("y: "); ImGui::SameLine(); ImGui::Text(TO_C_STR(up.y));
        ImGui::BulletText("z: "); ImGui::SameLine(); ImGui::Text(TO_C_STR(up.z));

        
    }
    ImGui::End();
}


#pragma endregion
