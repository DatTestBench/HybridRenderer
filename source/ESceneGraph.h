#ifndef ELITE_SCENE_GRAPH
#define ELITE_SCENE_GRAPH

//Project includes
#include <vector>
#include <map>
#include "EMesh.h"
namespace Elite
{
		enum RenderType
		{
			Color = 0,
			Depth = 1,
			RenderTypeSize = 2
		};

		enum RenderSystem
		{
			Software = 0,
			D3D = 1,
			RenderSystemSize = 2
		};
		class Camera;
		class SceneGraph
		{
		public:
			~SceneGraph();
			SceneGraph(const SceneGraph&) = delete;
			SceneGraph& operator= (const SceneGraph&) = delete;
			SceneGraph(SceneGraph&&) = delete;
			SceneGraph& operator= (SceneGraph&&) = delete;

			//Singleton Functionality
			static SceneGraph* GetInstance();
			void Destroy();

			//External Item Manipulation
			void AddObjectToGraph(Mesh* pObject, int sceneIdx);
			void AddScene(int sceneIdx);
			void SetCamera(const FPoint3& origin, uint32_t windowWidth = 640, uint32_t windowHeight = 480, float fovD = 45);
			void ChangeCameraResolution(uint32_t width, uint32_t height);

			//Workers
			void Update(float dT);
			void IncreaseScene();
			void DecreaseScene();
			void ToggleRenderType();
			void ToggleRenderSystem();
			void ToggleTransparancy();
			void ToggleObjectRotation();

			//Getters
			const std::vector<Mesh*>& GetObjects();
			const std::vector<Mesh*>& GetCurrentSceneObjects();
			Camera* GetCamera() const;
			const RenderType& GetRenderType() const;
			const RenderSystem& GetRenderSystem() const;
			bool IsTransparancyOn() const;
			size_t AmountOfScenes() const;
			size_t Size() const;
		private:
			SceneGraph();

			//Data Members
			static SceneGraph* m_pSceneGraph;
			std::vector<Mesh*> m_Objects;
			std::map<int, std::vector<Mesh*>> m_pScenes;
			static Camera* m_pCamera;

			//Scene Settings
			int m_CurrentScene;
			RenderType m_RenderType;
			RenderSystem m_RenderSystem;
			bool m_ShowTransparancy;
			bool m_AreObjectsRotating;

		};
}
#endif // !ELITE_SCENE_GRAPH
