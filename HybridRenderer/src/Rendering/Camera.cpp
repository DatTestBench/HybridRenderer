#include "pch.h"
#include "Rendering/Camera.hpp"
#include "Geometry/Mesh.hpp"
#include <SDL.h>

Camera::Camera(const glm::vec3& origin, const uint32_t windowWidth, const uint32_t windowHeight, const float fovD, const float nearPlane, const float farPlane)
// ELITE_OLD Camera::Camera(const Elite::FPoint3& origin, const uint32_t windowWidth, const uint32_t windowHeight, const float fovD, const float nearPlane, const float farPlane)
    : m_Origin{origin},
      m_RenderSystem{SceneGraph::GetInstance()->GetRenderSystem()},
      m_MovementSensitivity{60.f},
      m_RotationSensitivity{0.5f},
      m_Pitch{},
      m_Yaw{},
      m_Width{windowWidth},
      m_Height{windowHeight},
      m_AspectRatio{static_cast<float>(windowWidth) / windowHeight},
      m_FOV{tanf(fovD * static_cast<float>(E_TO_RADIANS) / 2.f)},
      m_NearPlane{nearPlane},
      m_FarPlane{farPlane},
      m_LookAt{},
      m_ProjectionMatrix{},
      m_ViewMatrix{}
{
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

#pragma region Workers
/*General*/
void Camera::Update(float dT)
{
    UpdateLookAtMatrix(dT);
}

void Camera::UpdateLookAtMatrix(float dT)
{
    glm::ivec2 dMove;
    // ELITE_OLD Elite::IVector2 dMove;

    const auto buttonMask = SDL_GetRelativeMouseState(&dMove.x, &dMove.y);
    const auto* keyState = SDL_GetKeyboardState(nullptr);

    glm::vec3 movement{};
    // ELITE_OLD Elite::FVector3 movement{};
    glm::vec3 worldMovement{};
    // ELITE_OLD Elite::FVector3 worldMovement{};

    //Vertical (Y) movement in world space
    if (buttonMask == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
    {
        //m_Origin.y -= m_MovementSensitivity * dMove.y;
        worldMovement.y -= m_MovementSensitivity * dMove.y;
    }
    //Vertical (Y) movement in local space
    if (buttonMask == (SDL_BUTTON_LMASK | SDL_BUTTON_MMASK))
    {
        movement.y += m_MovementSensitivity * dMove.y;
    }
    //Horizontal rotation + Horizontal movement (Z) in local space
    if (buttonMask == SDL_BUTTON_LMASK)
    {
        m_Yaw -= dMove.x * static_cast<float>(E_TO_RADIANS) * m_RotationSensitivity;
        movement.z += m_MovementSensitivity * dMove.y;
    }
    //Horizontal movement (X) in local space + Horizontal movement (Z) in local space
    if (buttonMask == SDL_BUTTON_MMASK)
    {
        movement.x += m_MovementSensitivity * dMove.x;
        movement.z += m_MovementSensitivity * dMove.y;
    }
    //Free cam rotation
    if (buttonMask == SDL_BUTTON_RMASK)
    {
        m_Yaw -= dMove.x * static_cast<float>(E_TO_RADIANS) * m_RotationSensitivity;
        m_Pitch -= dMove.y * static_cast<float>(E_TO_RADIANS) * m_RotationSensitivity;
    }

    //Keyboard movement (Y) in world space
    if (keyState[SDL_SCANCODE_E])
    {
        //m_Origin.y += m_MovementSensitivity * dT;
        worldMovement.y += m_MovementSensitivity;
    }
    if (keyState[SDL_SCANCODE_Q])
    {
        //m_Origin.y -= m_MovementSensitivity * dT;
        worldMovement.y -= m_MovementSensitivity;
    }


    //Keyboard movement (X/Z) in local space
    if (keyState[SDL_SCANCODE_W])
    {
        movement.z -= m_MovementSensitivity;
    }
    if (keyState[SDL_SCANCODE_S])
    {
        movement.z += m_MovementSensitivity;
    }
    if (keyState[SDL_SCANCODE_A])
    {
        movement.x -= m_MovementSensitivity;
    }
    if (keyState[SDL_SCANCODE_D])
    {
        movement.x += m_MovementSensitivity;
    }

    //Apply local space movement (m_LookAt needs to be transposed for D3D because...reasons??? - 
    //seemingly because vector transformation need the transposed inverse of the world matrix)
    //Some movement is still broken when y < 0

    switch (m_RenderSystem)
    {
    case Software:
        m_Origin += (glm::vec3(m_LookAt * glm::vec4(movement, 0)) + worldMovement) * dT;
        // ELITE_OLD m_Origin += (Elite::FVector3(m_LookAt * Elite::FVector4(movement)) + worldMovement) * dT;
        break;
    case D3D:
        m_Origin += (glm::vec3(glm::transpose(m_LookAt) * glm::vec4(movement, 0)) + worldMovement) * dT;
        // ELITE_OLD m_Origin += (Elite::FVector3(Elite::Transpose(m_LookAt) * Elite::FVector4(movement)) + worldMovement) * dT;
        break;
    default:
        break;
    }

    //Clamping pitch to prevent wonky behaviour when going over (or close to), 90 degrees.
    m_Pitch = glm::clamp(m_Pitch, glm::radians(-80.f), glm::radians(80.f));
    // ELITE_OLD m_Pitch = Elite::Clamp(m_Pitch, static_cast<float>(E_TO_RADIANS) * -80.f, static_cast<float>(E_TO_RADIANS) * 80.f);
    //Fmodding yaw to allow for rollover and 360 degree movement.
    m_Yaw = fmod(m_Yaw, glm::two_pi<float>());
    // ELITE_OLD m_Yaw = fmod(m_Yaw, static_cast<float>(E_PI_2));


    //FPS Camera approach adapted from https://www.3dgep.com/understanding-the-view-matrix/

    //Pure FPS camera
    /*float cosPitch = cos(m_Pitch);
    float sinPitch = sin(m_Pitch);
    float cosYaw = cos(m_Yaw);
    float sinYaw = sin(m_Yaw);

    FVector4 forward{ - sinYaw * cosPitch, sinPitch, cosPitch * cosYaw };
    FVector4 right{ cosYaw, 0, -sinYaw };
    FVector4 up{ sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
    FVector4 transXRot{ -Dot(right, FVector4(m_Origin)), -Dot(up, FVector4(m_Origin)), Dot(forward, FVector4(m_Origin)), 1 };
    m_LookAt = FMatrix4(right, up, forward, transXRot);*/


    //Using intermediate approach as flipping the x and y of the forward vector also has to affect the right and up vector
    //FPS Camera for Forward, rest is lookat construction
    const auto cosPitch = cos(m_Pitch);
    const auto sinPitch = sin(m_Pitch);
    const auto cosYaw = cos(m_Yaw);
    const auto sinYaw = sin(m_Yaw);

    glm::vec4 forward{};
    // ELITE_OLD Elite::FVector4 forward{};
    glm::vec4 worldUp{0, 1, 0, 0};
    // ELITE_OLD Elite::FVector4 worldUp{0, 1, 0, 0};
    glm::vec4 right{};
    // ELITE_OLD Elite::FVector4 right{};
    glm::vec4 up{};
    // ELITE_OLD Elite::FVector4 up{};

    switch (m_RenderSystem)
    {
    case Software:

        forward = {sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw, 0};
        // ELITE_OLD forward = {sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw};
        right = {glm::normalize(glm::cross(glm::vec3(worldUp), glm::vec3(forward))), 0.f};
        // ELITE_OLD right = {Elite::GetNormalized(Elite::Cross(worldUp.xyz, forward.xyz)), 0.f};
        up = {glm::normalize(glm::cross(glm::vec3(forward), glm::vec3(right))), 0.f};
        // ELITE_OLD up = {Elite::GetNormalized(Elite::Cross(forward.xyz, right.xyz)), 0.f};

        m_LookAt = glm::mat4(right, up, forward, glm::vec4(m_Origin.x, m_Origin.y, m_Origin.z, 1.f));
        // ELITE_OLD m_LookAt = Elite::FMatrix4(right, up, forward, Elite::FVector4(m_Origin.x, m_Origin.y, m_Origin.z, 1.f));

        break;
    case D3D:

        forward = {-sinYaw * cosPitch, sinPitch, cosPitch * cosYaw, 0};
        // ELITE_OLD forward = {-sinYaw * cosPitch, sinPitch, cosPitch * cosYaw};
        right = {glm::normalize(glm::cross(glm::vec3(worldUp), glm::vec3(forward))), 0.f};
        // ELITE_OLD right = {Elite::GetNormalized(Elite::Cross(worldUp.xyz, forward.xyz)), 0.f};
        up = {glm::normalize(glm::cross(glm::vec3(forward), glm::vec3(right))), 0.f};
        // ELITE_OLD up = {Elite::GetNormalized(Elite::Cross(forward.xyz, right.xyz)), 0.f};
        m_LookAt = glm::mat4(right, up, forward, glm::vec4(m_Origin.x, m_Origin.y, -m_Origin.z, 1.f));
        // ELITE_OLD m_LookAt = Elite::FMatrix4(right, up, forward, Elite::FPoint4(m_Origin.x, m_Origin.y, -m_Origin.z, 1.f));
        break;
    default:
        break;
    }

    //FPS Camera for everything, lookat for final step, not used anymore as the flipping of the x and y in the forward vector -
    //also need to affect right and up vector creation
    /*float cosPitch = cos(m_Pitch);
    float sinPitch = sin(m_Pitch);
    float cosYaw = cos(m_Yaw);
    float sinYaw = sin(m_Yaw);

    FVector4 forward{-sinYaw * cosPitch, sinPitch, cosPitch * cosYaw };
    FVector4 right{ cosYaw, 0, -sinYaw };
    FVector4 up{ sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
    m_LookAt = FMatrix4(right, up, forward, FVector4(m_Origin.x, m_Origin.y, -m_Origin.z, 1.f));*/
}

/*Software*/

void Camera::MakeScreenSpace(Mesh* pMesh) const
{
    std::vector<VertexOutput> sSVertices;

    const auto viewProjWorldMatrix = m_ProjectionMatrix * glm::inverse(m_LookAt) * pMesh->GetWorld();
    // ELITE_OLD const auto viewProjWorldMatrix = m_ProjectionMatrix * Inverse(m_LookAt) * pMesh->GetWorld();

    for (auto& v : pMesh->GetVertices())
    {
        VertexOutput sSV{};
        sSV.uv = v.uv;
        sSV.worldPos = v.pos;

        sSV.pos = viewProjWorldMatrix * glm::vec4(v.pos, 1);
        // ELITE_OLD sSV.pos = viewProjWorldMatrix * Elite::FPoint4(v.pos);
        sSV.normal = glm::vec3(pMesh->GetWorld() * glm::vec4(v.normal, 0));
        // ELITE_OLD sSV.normal = Elite::FVector3(pMesh->GetWorld() * Elite::FVector4(v.normal));
        sSV.tangent = glm::vec3(pMesh->GetWorld() * glm::vec4(v.tangent, 0));
        // ELITE_OLD sSV.tangent = Elite::FVector3(pMesh->GetWorld() * Elite::FVector4(v.tangent));

        sSV.pos.x /= sSV.pos.w;
        sSV.pos.y /= sSV.pos.w;
        sSV.pos.z /= sSV.pos.w;

        sSV.viewDirection = glm::normalize((glm::mat3(pMesh->GetWorld()) * v.pos) - m_Origin);
        // ELITE_OLD sSV.viewDirection = GetNormalized((Elite::FMatrix3(pMesh->GetWorld()) * v.pos) - m_Origin);
        if (sSV.pos.x < -1 || sSV.pos.x > 1 || sSV.pos.y < -1 || sSV.pos.y > 1 || sSV.pos.z < 0 || sSV.pos.z > 1)
        {
            sSV.culled = true;
        }
        else
        {
            sSV.culled = false;
        }

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
    // ELITE_OLD m_FOV = tanf(fovD * static_cast<float>(E_TO_RADIANS) / 2.f);
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
