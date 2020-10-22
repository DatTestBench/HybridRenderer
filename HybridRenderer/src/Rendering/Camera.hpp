#ifndef CAMERA_HPP
#define CAMERA_HPP

//Project includes
#include "Helpers/EMath.h"
#include "Scene/SceneGraph.hpp"

class Mesh;

class Camera
{
public:
    Camera(const Elite::FPoint3& origin, uint32_t windowWidth, uint32_t windowHeight, float fovD, float nearPlane = 0.1f, float farPlane = 100.f);
    ~Camera() = default;
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) = delete;
    Camera& operator=(Camera&&) = delete;

    //Workers
    void Update(float dT);
    void MakeScreenSpace(Mesh* pMesh) const;
    //Setters
    void SetResolution(uint32_t width, uint32_t height);
    void SetFOV(float fovD);
    void ToggleRenderSystem(const RenderSystem& renderSystem);

    //Getters
    Elite::FMatrix4 GetInverseViewMatrix() const;
    Elite::FMatrix4 GetViewMatrix() const;
    Elite::FMatrix4 GetProjectionMatrix() const;

private:
    Elite::FPoint3 m_Origin;
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
    Elite::FMatrix4 m_LookAt;
    Elite::FMatrix4 m_ProjectionMatrix;
    Elite::FMatrix4 m_ViewMatrix;

    void UpdateLookAtMatrix(float dT);
};
#endif // !ELITE_CAMERA
