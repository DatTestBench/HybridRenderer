#ifndef CAMERA_HPP
#define CAMERA_HPP

//Project includes
#include "Helpers/EMath.h"
#include "Scene/SceneGraph.hpp"

class Mesh;

class Camera
{
public:
    Camera(const glm::vec3& origin, uint32_t windowWidth, uint32_t windowHeight, float fovD, float nearPlane = 0.1f, float farPlane = 100.f);
    // ELITE_OLD Camera(const Elite::FPoint3& origin, uint32_t windowWidth, uint32_t windowHeight, float fovD, float nearPlane = 0.1f, float farPlane = 100.f);
    ~Camera() = default;
    
    DEL_ROF(Camera)

    //Workers
    void Update(float dT);
    void MakeScreenSpace(Mesh* pMesh) const;
    //Setters
    void SetResolution(uint32_t width, uint32_t height);
    void SetFOV(float fovD);
    void ToggleRenderSystem(const RenderSystem& renderSystem);

    //Getters

    [[nodiscard]] auto GetInverseViewMatrix() const noexcept -> glm::mat4 { return m_LookAt; }
    // ELITE_OLD [[nodiscard]] auto GetInverseViewMatrix() const noexcept -> Elite::FMatrix4 { return m_LookAt; }
    [[nodiscard]] auto GetViewMatrix() const noexcept -> glm::mat4 { return glm::inverse(m_LookAt); }
    // ELITE_OLD [[nodiscard]] auto GetViewMatrix() const noexcept -> Elite::FMatrix4 { return Inverse(m_LookAt); }
    [[nodiscard]] auto GetProjectionMatrix() const noexcept -> glm::mat4 { return m_ProjectionMatrix; }
    // ELITE_OLD [[nodiscard]] auto GetProjectionMatrix() const noexcept -> Elite::FMatrix4 { return m_ProjectionMatrix; }

private:
    glm::vec3 m_Origin;
    // ELITE_OLD Elite::FPoint3 m_Origin;
    RenderSystem m_RenderSystem;
    //Movement variables
    float m_MovementSensitivity;
    float m_RotationSensitivity;

    float m_Pitch;
    float m_Yaw;

    //Display settings
    uint32_t m_Width;
    uint32_t m_Height;
    float m_AspectRatio;
    float m_FOV;
    float m_NearPlane;
    float m_FarPlane;

    //Matrices
    glm::mat4 m_LookAt;
    // ELITE_OLD Elite::FMatrix4 m_LookAt;
    glm::mat4 m_ProjectionMatrix;
    // ELITE_OLD Elite::FMatrix4 m_ProjectionMatrix;
    glm::mat4 m_ViewMatrix;
    // ELITE_OLD Elite::FMatrix4 m_ViewMatrix;

    void UpdateLookAtMatrix(float dT);
};
#endif // !ELITE_CAMERA
