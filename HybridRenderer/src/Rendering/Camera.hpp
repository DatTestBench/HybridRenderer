#ifndef CAMERA_HPP
#define CAMERA_HPP

//Project includes
#include "Scene/SceneGraph.hpp"

class Mesh;

class Camera
{
public:
    explicit Camera(const glm::vec3& origin, uint32_t windowWidth, uint32_t windowHeight, float fovD, float nearPlane = 0.1f, float farPlane = 100.f);
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
    [[nodiscard]] auto GetInverseViewMatrix() const noexcept -> glm::mat4 { return glm::inverse(m_CameraMatrix); }
    [[nodiscard]] auto GetViewMatrix() const noexcept -> glm::mat4 { return m_CameraMatrix; }
    [[nodiscard]] auto GetProjectionMatrix() const noexcept -> glm::mat4 { return m_ProjectionMatrix; }
    [[nodiscard]] constexpr auto GetPosition() const noexcept -> glm::vec3 { return m_Origin; }
    [[nodiscard]] constexpr auto GetPitch() const noexcept -> float { return m_Pitch; }
    [[nodiscard]] constexpr auto GetYaw() const noexcept -> float { return m_Yaw; }

    [[nodiscard]] constexpr auto GetForward() const noexcept -> glm::vec3 { return m_Forward; }
    [[nodiscard]] constexpr auto GetRight() const noexcept -> glm::vec3 { return m_Right; }
    [[nodiscard]] constexpr auto GetUp() const noexcept -> glm::vec3 { return m_Up; }

private:
    glm::vec3 m_Origin;
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
    glm::mat4 m_CameraMatrix;
    glm::mat4 m_ProjectionMatrix;
    glm::mat4 m_ViewMatrix;

    glm::vec3 m_Forward;
    glm::vec3 m_Right;
    glm::vec3 m_Up;

    void UpdateLookAtMatrix(float dT);
};
#endif // !CAMERA_HPP
