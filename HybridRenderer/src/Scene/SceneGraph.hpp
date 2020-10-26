#ifndef SCENE_GRAPH_HPP
#define SCENE_GRAPH_HPP

//Standard Includes
#include <vector>
#include <map>

//Project includes
#include "Geometry/Mesh.hpp"
#include "Helpers/Singleton.hpp"

class Timer;
class Renderer;

enum RenderType
{
    Color = 0,
    Depth = 1
};

enum RenderSystem
{
    Software = 0,
    D3D = 1
};

class Camera;

class SceneGraph final : public Singleton<SceneGraph>
{
public:
    explicit SceneGraph(Token) : m_pTimer(nullptr), m_CurrentScene(0), m_RenderType(Color), m_RenderSystem(Software), m_ShowTransparency(true), m_AreObjectsRotating(true), m_ShouldUpdateRenderSystem(false){}
    ~SceneGraph();

    DEL_ROF(SceneGraph)

    //External Item Manipulation
    void AddObjectToGraph(Mesh* pObject, int sceneIdx);
    void AddScene(uint32_t sceneIdx);
    static void SetCamera(const glm::vec3& origin, uint32_t windowWidth = 640, uint32_t windowHeight = 480, float fovD = 45);
    static void ChangeCameraResolution(uint32_t width, uint32_t height);

    //Workers
    void Update(float dT);
    void RenderDebugUI() noexcept;
    void SetTimer(Timer* pTimer) noexcept { m_pTimer = pTimer; };
    void ConfirmRenderSystemUpdate() noexcept { m_ShouldUpdateRenderSystem = false; }

    //Getters
    [[nodiscard]] constexpr auto GetObjects() const noexcept -> const std::vector<Mesh*>& { return m_Objects; }
    [[nodiscard]] auto GetCurrentSceneObjects() const noexcept -> const std::vector<Mesh*>& { return m_pScenes.at(m_CurrentScene); }
    [[nodiscard]] static constexpr auto GetCamera() noexcept -> Camera* { return m_pCamera; }
    [[nodiscard]] constexpr auto GetRenderType() const noexcept -> RenderType { return m_RenderType; }
    [[nodiscard]] constexpr auto GetRenderSystem() const noexcept -> RenderSystem { return m_RenderSystem; }
    [[nodiscard]] constexpr auto IsTransparencyOn() const noexcept -> bool { return m_ShowTransparency; }
    [[nodiscard]] auto AmountOfScenes() const noexcept -> uint32_t { return static_cast<uint32_t>(m_pScenes.size()); }
    [[nodiscard]] auto AmountOfObjects() const noexcept -> uint32_t { return static_cast<uint32_t>(m_Objects.size()); }
    [[nodiscard]] constexpr auto ShouldUpdateRenderSystem() const noexcept -> bool { return m_ShouldUpdateRenderSystem; }
private:
    //Data Members
    std::vector<Mesh*> m_Objects;
    std::map<int, std::vector<Mesh*>> m_pScenes;
    static Camera* m_pCamera;
    Timer* m_pTimer;
    //Scene Settings
    uint32_t m_CurrentScene;
    RenderType m_RenderType;
    RenderSystem m_RenderSystem;
    bool m_ShowTransparency;
    bool m_AreObjectsRotating;
    bool m_ShouldUpdateRenderSystem;
};

#endif // !SCENE_GRAPH_HPP
