#ifndef SCENE_GRAPH_HPP
#define SCENE_GRAPH_HPP

//Standard Includes
#include <vector>
#include <map>

//Project includes
#include "Helpers/Singleton.hpp"

class Timer;
class Renderer;
class Mesh;

enum class SoftwareRenderType
{
    Color = 0,
    Depth = 1,
    Normal = 2,
    NormalMapped = 3
};

enum class HardwareRenderType
{
    Color = 0,
    Specular = 1,
    Normal = 2,
    NormalMapped = 3
};

enum class HardwareFilterType
{
    Point = 0,
    Linear = 1,
    Anisotropic = 2
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
    explicit SceneGraph(Token)
    : m_pTimer(nullptr)
    , m_CurrentScene(0)
    , m_SoftwareRenderType(SoftwareRenderType::Color)
    , m_HardwareRenderType(HardwareRenderType::Color)
    , m_HardwareFilterType(HardwareFilterType::Point)
    , m_RenderSystem(Software)
    , m_ShowTransparency(true)
    , m_AreObjectsRotating(false)
    , m_ShouldUpdateRenderSystem(false)
    , m_ShouldUpdateHardwareTypes(false)
    , m_RenderRTFrame(false)
    , m_ShowRTRender(false){}
    ~SceneGraph();

    DEL_ROF(SceneGraph)

    //External Item Manipulation
    void AddObjectToGraph(Mesh* pObject, int sceneIdx);
    void AddScene(uint32_t sceneIdx);
    static void SetCamera(const glm::vec3& origin, uint32_t windowWidth = 640, uint32_t windowHeight = 480, float fovD = 45);
    static void ChangeCameraResolution(uint32_t width, uint32_t height);

    //Workers
    void Update(float dT) const;
    void RenderDebugUI() noexcept;
    void SetTimer(Timer* pTimer) noexcept { m_pTimer = pTimer; }
    void ConfirmRenderSystemUpdate() noexcept { m_ShouldUpdateRenderSystem = false; }
    void ConfirmHardwareTypesUpdate() noexcept { m_ShouldUpdateHardwareTypes = false; }
    void ConfirmRTRender() noexcept { m_RenderRTFrame = false; }

    //Getters
    [[nodiscard]] constexpr auto GetObjects() const noexcept -> const std::vector<Mesh*>& { return m_Objects; }
    [[nodiscard]] auto GetCurrentSceneObjects() const noexcept -> const std::vector<Mesh*>& { return m_pScenes.at(m_CurrentScene); }
    [[nodiscard]] static constexpr auto GetCamera() noexcept -> Camera* { return m_pCamera; }
    [[nodiscard]] constexpr auto GetSoftwareRenderType() const noexcept -> SoftwareRenderType { return m_SoftwareRenderType; }
    [[nodiscard]] constexpr auto GetHardwareRenderType() const noexcept -> HardwareRenderType { return m_HardwareRenderType; }
    [[nodiscard]] constexpr auto GetHardwareFilterType() const noexcept -> HardwareFilterType { return m_HardwareFilterType; }
    [[nodiscard]] constexpr auto GetRenderSystem() const noexcept -> RenderSystem { return m_RenderSystem; }
    [[nodiscard]] constexpr auto IsTransparencyOn() const noexcept -> bool { return m_ShowTransparency; }
    [[nodiscard]] auto AmountOfScenes() const noexcept -> uint32_t { return static_cast<uint32_t>(m_pScenes.size()); }
    [[nodiscard]] auto AmountOfObjects() const noexcept -> uint32_t { return static_cast<uint32_t>(m_Objects.size()); }
    [[nodiscard]] constexpr auto ShouldUpdateRenderSystem() const noexcept -> bool { return m_ShouldUpdateRenderSystem; }
    [[nodiscard]] constexpr auto ShouldUpdateHardwareTypes() const noexcept -> bool { return m_ShouldUpdateHardwareTypes; }
    [[nodiscard]] constexpr auto ShouldRenderRTFrame() const noexcept -> bool { return m_RenderRTFrame; }
    [[nodiscard]] constexpr auto ShouldShowRTRender() const noexcept -> bool { return m_ShowRTRender; }
private:
    //Data Members
    std::vector<Mesh*> m_Objects;
    std::map<uint32_t, std::vector<Mesh*>> m_pScenes;
    static Camera* m_pCamera;
    Timer* m_pTimer;
    //Scene Settings
    uint32_t m_CurrentScene;
    SoftwareRenderType m_SoftwareRenderType;
    HardwareRenderType m_HardwareRenderType;
    HardwareFilterType m_HardwareFilterType;
    RenderSystem m_RenderSystem;
    bool m_ShowTransparency;
    bool m_AreObjectsRotating;
    bool m_ShouldUpdateRenderSystem;
    bool m_ShouldUpdateHardwareTypes;
    bool m_RenderRTFrame;
    bool m_ShowRTRender;

    void RenderSoftwareDebugUI() noexcept;
    void RenderHardwareDebugUI() noexcept;

};

#endif // !SCENE_GRAPH_HPP
