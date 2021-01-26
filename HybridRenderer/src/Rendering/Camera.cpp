#include "pch.h"
#include "Rendering/Camera.hpp"

#include "Geometry/Mesh.hpp"
#include <SDL.h>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/euler_angles.hpp>

Camera::Camera(const glm::vec3& origin, const uint32_t windowWidth, const uint32_t windowHeight, const float fovD, const float nearPlane, const float farPlane)
    : m_Origin{origin},
      m_RenderSystem{SceneGraph::GetInstance()->GetRenderSystem()},
      m_MovementSensitivity{60.f},
      m_RotationSensitivity{0.5f},
      m_Pitch{},
      m_Yaw{},
      m_Width{windowWidth},
      m_Height{windowHeight},
      m_AspectRatio{static_cast<float>(windowWidth) / windowHeight},
      m_FOV{tanf(glm::radians(fovD) / 2.f)},
      m_NearPlane{nearPlane},
      m_FarPlane{farPlane},
      m_CameraMatrix{},
      m_ProjectionMatrix{},
      m_ViewMatrix{},
      m_Forward(0),
      m_Right(0),
      m_Up(0)
{
    switch (m_RenderSystem)
    {
    case Software:
        m_ProjectionMatrix = {
            {1.f / (m_AspectRatio * m_FOV), 0, 0, 0},
            {0, 1.f / m_FOV, 0, 0},
            {0, 0, m_FarPlane / (m_NearPlane - m_FarPlane), -1},
            {0, 0, (m_FarPlane * m_NearPlane) / (m_NearPlane - m_FarPlane), 0}
        };
        break;
    case D3D:
        m_ProjectionMatrix = {
            {1.f / (m_AspectRatio * m_FOV), 0, 0, 0},
            {0, 1.f / m_FOV, 0, 0},
            {0, 0, m_FarPlane / (m_FarPlane - m_NearPlane), 1},
            {0, 0, -(m_NearPlane * m_FarPlane) / (m_FarPlane - m_NearPlane), 0}
        };
        break;
    default:
        break;
    }
}

#pragma region Workers
/*General*/
void Camera::Update(const float dT)
{
    UpdateLookAtMatrix(dT);
}

void Camera::UpdateLookAtMatrix(float dT)
{
    glm::ivec2 dMove;

    const auto buttonMask = SDL_GetRelativeMouseState(&dMove.x, &dMove.y);
    const auto* keyState = SDL_GetKeyboardState(nullptr);

    glm::vec3 movement{};
    glm::vec3 worldMovement{};

    // Don't update the movement if we're hovering over any ImGui UI (this is not an ideal place to do this, but it works
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        //Vertical (Y) movement in world space
        if (buttonMask == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
        {
            //m_Origin.y -= m_MovementSensitivity * dMove.y;
            worldMovement.y -= m_MovementSensitivity * dMove.y * dT;
        }
        //Vertical (Y) movement in local space
        if (buttonMask == (SDL_BUTTON_LMASK | SDL_BUTTON_MMASK))
        {
            movement.y += m_MovementSensitivity * dMove.y * dT;
        }
        //Horizontal rotation + Horizontal movement (Z) in local space
        if (buttonMask == SDL_BUTTON_LMASK)
        {
            m_Yaw += glm::radians(dMove.x * m_RotationSensitivity);
            movement.z += m_MovementSensitivity * dMove.y * dT;
        }
        //Horizontal movement (X) in local space + Horizontal movement (Z) in local space
        if (buttonMask == SDL_BUTTON_MMASK)
        {
            movement.x += m_MovementSensitivity * dMove.x * dT;
            movement.z += m_MovementSensitivity * dMove.y * dT;
        }
        //Free cam rotation
        if (buttonMask == SDL_BUTTON_RMASK)
        {
            m_Yaw += glm::radians(dMove.x * m_RotationSensitivity);
            m_Pitch += glm::radians(dMove.y * m_RotationSensitivity);
        }

        //Keyboard movement (Y) in world space
        if (keyState[SDL_SCANCODE_E])
        {
            //m_Origin.y += m_MovementSensitivity * dT;
            worldMovement.y += m_MovementSensitivity * dT;
        }
        if (keyState[SDL_SCANCODE_Q])
        {
            //m_Origin.y -= m_MovementSensitivity * dT;
            worldMovement.y -= m_MovementSensitivity * dT;
        }

        auto forward = m_Forward;
        //if (m_RenderSystem == Software)
            //forward *= -1;
        
        //Keyboard movement (X/Z) in local space
        if (keyState[SDL_SCANCODE_W])
        {
            movement += forward * m_MovementSensitivity * dT;
            //movement.z -= m_MovementSensitivity;
        }
        if (keyState[SDL_SCANCODE_S])
        {
            movement -= forward * m_MovementSensitivity * dT;
            //movement.z += m_MovementSensitivity;
        }
        if (keyState[SDL_SCANCODE_A])
        {
            movement -= m_Right * m_MovementSensitivity * dT;
            //movement.x -= m_MovementSensitivity;
        }
        if (keyState[SDL_SCANCODE_D])
        {
            movement+= m_Right * m_MovementSensitivity * dT;
            //movement.x += m_MovementSensitivity;
        }
    }
    //Apply local space movement (m_LookAt needs to be transposed for D3D because...reasons??? - 
    //seemingly because vector transformation need the transposed inverse of the world matrix)
    //Some movement is still broken when y < 0
    //switch (m_RenderSystem)
    //{
    //case Software:
    //    //m_Origin += (glm::vec3(glm::vec4(movement, 0)) + worldMovement) * dT;
    //    m_Origin += (glm::vec3(m_CameraMatrix * glm::vec4(movement, 0)) + worldMovement) * dT;
    //    break;
    //case D3D:
    //    //m_Origin += (glm::vec3(glm::vec4(movement, 0)) + worldMovement) * dT;
    //    m_Origin += (glm::vec3((m_CameraMatrix * -1.f) * glm::vec4(movement, 0)) + worldMovement) * dT;
    //    break;
    //default:
    //    break;
    //}

    m_Origin += movement + worldMovement;

    //Clamping pitch to prevent wonky behaviour when going over (or close to), 90 degrees.
    m_Pitch = glm::clamp(m_Pitch, glm::radians(-80.f), glm::radians(80.f));
    //Fmodding yaw to allow for rollover and 360 degree movement.
    m_Yaw = fmod(m_Yaw, glm::two_pi<float>());


    //FPS Camera approach adapted from https://www.3dgep.com/understanding-the-view-matrix/

    //Using intermediate approach as flipping the x and y of the forward vector also has to affect the right and up vector
    //FPS Camera for Forward, rest is lookat construction
    const auto cosPitch = cos(m_Pitch);
    const auto sinPitch = sin(m_Pitch);
    const auto cosYaw = cos(m_Yaw);
    const auto sinYaw = sin(m_Yaw);

    glm::vec3 worldUp{0, 1, 0};

    // todo this whole things doesn't really make sense. The cross products should be inverted for D3D, using glm::lookatRH/lookatLH should work, but doesn't. For now it works ish, but this is not how it should be
    switch (m_RenderSystem)
    {
    case Software:
        {
            m_Forward = -glm::normalize(glm::vec3(-sinYaw * cosPitch, sinPitch, cosPitch * cosYaw));
            /*m_Right = glm::normalize(glm::cross(worldUp, m_Forward));
            m_Up = glm::normalize(glm::cross(m_Forward, m_Right));
            
            m_CameraMatrix = glm::mat4({m_Right, 0},
                                       {m_Up, 0},
                                       {m_Forward, 0},
                                       glm::vec4(m_Origin.x, m_Origin.y, m_Origin.z, 1.f));*/


            m_CameraMatrix = glm::lookAtRH(m_Origin, m_Origin + m_Forward, worldUp);
            
            m_Right = glm::row(m_CameraMatrix, 0);
            m_Up = glm::row(m_CameraMatrix, 1);
            m_Forward = -glm::row(m_CameraMatrix, 2);
            break;
        }
    case D3D:
        {
            m_Forward = glm::normalize(glm::vec3(sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw));

            /*
            m_Right = glm::normalize(glm::cross(worldUp, m_Forward));
            m_Up = glm::normalize(glm::cross(m_Forward, m_Right));

            m_CameraMatrix = glm::mat4({m_Right, 0},
                                       {m_Up, 0},
                                       {m_Forward, 0},
                                       glm::vec4(m_Origin.x, m_Origin.y, -m_Origin.z, 1.f));
                                       */
            auto tempO = m_Origin;
            tempO.z = -tempO.z;
            
            m_CameraMatrix =glm::lookAtLH(tempO, tempO + m_Forward, worldUp);

            m_Right = glm::row((m_CameraMatrix), 0);
            m_Up = glm::row((m_CameraMatrix), 1);
            m_Forward = glm::row((m_CameraMatrix), 2);

            m_Right.z = -m_Right.z;
            m_Up.z = -m_Up.z;
            m_Forward.z = -m_Forward.z;
            break;
        }
    default:
        break;
    }
}

/*Software*/

void Camera::MakeScreenSpace(Mesh* pMesh) const
{
    std::vector<VertexOutput> sSVertices;

    const auto meshWorld = pMesh->GetWorld();
    const auto meshWorld3 = glm::mat3(meshWorld);
    const auto viewProjWorldMatrix = m_ProjectionMatrix * m_CameraMatrix * meshWorld;

    
    
    for (const auto& v : pMesh->GetVertices())
    {
        
        VertexOutput sSV{};
        sSV.uv = v.uv;
        sSV.worldPos = meshWorld * glm::vec4(v.pos, 1.f);

        sSV.pos = viewProjWorldMatrix * glm::vec4(v.pos, 1.f);
        sSV.normal = v.normal * meshWorld3;
        sSV.tangent = v.tangent * meshWorld3;

        // Perspective transform
        sSV.pos.x /= sSV.pos.w;
        sSV.pos.y /= sSV.pos.w;
        sSV.pos.z /= sSV.pos.w;

        sSV.viewDirection = glm::normalize(meshWorld * glm::vec4(v.pos, 1.f) - glm::vec4(m_Origin.x, m_Origin.y, -m_Origin.z, 1.f));
        if (sSV.pos.x < -1 || sSV.pos.x > 1
            || sSV.pos.y < -1 || sSV.pos.y > 1
            || sSV.pos.z < 0 || sSV.pos.z > 1)
        {
            sSV.culled = true;
        }
        else
        {
            sSV.culled = false;
        }

    
        // Convert to screenspace    
        sSV.pos.x = (sSV.pos.x + 1) / 2 * m_Width;
        sSV.pos.y = (1 - sSV.pos.y) / 2 * m_Height;
        sSVertices.push_back(sSV);
    }
    pMesh->SetScreenSpaceVertices(sSVertices);
}
#pragma endregion

#pragma region Setters
void Camera::SetResolution(const uint32_t width, const uint32_t height)
{
    m_Width = width;
    m_Height = height;
    m_AspectRatio = static_cast<float>(width) / height;
}

void Camera::SetFOV(const float fovD)
{
    m_FOV = glm::tan(glm::radians(fovD) / 2.f);
}

void Camera::ToggleRenderSystem(const RenderSystem& renderSystem)
{
    m_RenderSystem = renderSystem;
    switch (m_RenderSystem)
    {
    case Software:
        m_ProjectionMatrix = {
            {1.f / (m_AspectRatio * m_FOV), 0, 0, 0},
            {0, 1.f / m_FOV, 0, 0},
            {0, 0, m_FarPlane / (m_NearPlane - m_FarPlane), - 1},
            {0, 0, (m_FarPlane * m_NearPlane) / (m_NearPlane - m_FarPlane), 0}
        };
        break;
    case D3D:
        m_ProjectionMatrix = {
            {1.f / (m_AspectRatio * m_FOV), 0, 0, 0},
            {0, 1.f / m_FOV, 0, 0},
            {0, 0, m_FarPlane / (m_FarPlane - m_NearPlane), 1},
            {0, 0, -(m_NearPlane * m_FarPlane) / (m_FarPlane - m_NearPlane), 0}
        };
        break;
    default:
        break;
    }
}
#pragma endregion
