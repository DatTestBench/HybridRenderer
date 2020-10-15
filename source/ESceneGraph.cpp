#include "pch.h"
#include <iostream>
#include <string>
#include "SDL.h"
#include "ESceneGraph.h"
#include "ECamera.h"

Elite::SceneGraph* Elite::SceneGraph::m_pSceneGraph = nullptr;
Elite::Camera* Elite::SceneGraph::m_pCamera = nullptr;

Elite::SceneGraph::SceneGraph()
	: m_Objects{}
	, m_CurrentScene{ 0 }
	, m_RenderType{ RenderType::Color }
	, m_RenderSystem{ RenderSystem::Software }
	, m_ShowTransparancy{ true }
	, m_AreObjectsRotating{ true }
{
}

Elite::SceneGraph::~SceneGraph()
{
	for (size_t i = 0; i < Size(); i++)
	{
		delete m_Objects[i];
		m_Objects[i] = nullptr;
	}

	delete m_pCamera;
}

#pragma region SingletonFunctionality
Elite::SceneGraph* Elite::SceneGraph::GetInstance()
{
	if (m_pSceneGraph == nullptr)
		m_pSceneGraph = new SceneGraph();
	return m_pSceneGraph;
}

void Elite::SceneGraph::Destroy()
{
	delete SceneGraph::GetInstance();
}
#pragma endregion

#pragma region ExternalItemManipulation
void Elite::SceneGraph::AddObjectToGraph(Mesh* pObject, int sceneIdx)
{
	m_Objects.push_back(pObject);
	m_pScenes.at(sceneIdx).push_back(pObject);
}

void Elite::SceneGraph::AddScene(int sceneIdx)
{
	m_pScenes.try_emplace(sceneIdx);
}

void Elite::SceneGraph::SetCamera(const FPoint3& origin, uint32_t windowWidth, uint32_t windowHeight, float fovD)
{
	if (m_pCamera == nullptr)
		m_pCamera = new Camera(origin, windowWidth, windowHeight, fovD);
}

void Elite::SceneGraph::ChangeCameraResolution(uint32_t width, uint32_t height)
{
	if (m_pCamera == nullptr)
		return;
	m_pCamera->SetResolution(width, height);
}
#pragma endregion

#pragma region Workers
void Elite::SceneGraph::Update(float dT)
{
	float rotationSpeed{};
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

void Elite::SceneGraph::IncreaseScene()
{
	m_CurrentScene = (m_CurrentScene == m_pScenes.size() - 1) ? 0 : m_CurrentScene + 1;
	std::cout << "Scene " << m_CurrentScene + 1 << " / " << m_pScenes.size() << std::endl;
}

void Elite::SceneGraph::DecreaseScene()
{
	m_CurrentScene = (m_CurrentScene == 0) ? int(m_pScenes.size()) - 1 : m_CurrentScene - 1;
	std::cout << "Scene " << m_CurrentScene + 1 << " / " << m_pScenes.size() << std::endl;
}

void Elite::SceneGraph::ToggleRenderType()
{
	if (m_RenderSystem == RenderSystem::D3D)
	{
		std::cout << "RenderType can not be changed in D3D mode\n";
		return;
	}

	m_RenderType = (m_RenderType == RenderType::RenderTypeSize - 1) ? RenderType(0) : RenderType(m_RenderType + 1);
	
	std::cout << "Rendertype changed to ";
	switch (m_RenderType)
	{
	case RenderType::Color:
		std::cout << "show color\n";
		break;
	case RenderType::Depth:
		std::cout << "show depthbuffer\n";
		break;
	default:
		break;
	}
}

void Elite::SceneGraph::ToggleRenderSystem()
{
	m_RenderSystem = (m_RenderSystem == RenderSystem::RenderSystemSize - 1) ? RenderSystem(0) : RenderSystem(m_RenderSystem + 1);
	std::cout << "Rendersystem changed to ";
	switch (m_RenderSystem)
	{
	case RenderSystem::Software:
		std::cout << "software\n";
		break;
	case RenderSystem::D3D:
		std::cout << "D3D\n";
		break;
	default:
		break;
	}
	m_pCamera->ToggleRenderSystem(m_RenderSystem);
}

void Elite::SceneGraph::ToggleTransparancy()
{
	if (m_RenderSystem == RenderSystem::Software)
	{
		std::cout << "Transparancy toggle not available in software mode\n";
		return;
	}

	m_ShowTransparancy = !m_ShowTransparancy;

	if (m_ShowTransparancy)
	{
		std::cout << "Transparancy on\n";
	}
	else
	{
		std::cout << "Transparancy off\n";
	}
}

void Elite::SceneGraph::ToggleObjectRotation()
{
	m_AreObjectsRotating = !m_AreObjectsRotating;

	if (m_AreObjectsRotating)
	{
		std::cout << "Object rotation turned on\n";
	}
	else
	{
		std::cout << "Object rotation turned off\n";
	}
}
#pragma endregion

#pragma region Getters
const std::vector<Elite::Mesh*>& Elite::SceneGraph::GetObjects()
{
	return m_Objects;
}

const std::vector<Elite::Mesh*>& Elite::SceneGraph::GetCurrentSceneObjects()
{
	return m_pScenes.at(m_CurrentScene);
}

Elite::Camera* Elite::SceneGraph::GetCamera() const
{
	return m_pCamera;
}

const Elite::RenderType& Elite::SceneGraph::GetRenderType() const
{
	return m_RenderType;
}

const Elite::RenderSystem& Elite::SceneGraph::GetRenderSystem() const
{
	return m_RenderSystem;
}

bool Elite::SceneGraph::IsTransparancyOn() const
{
	return m_ShowTransparancy;
}

size_t Elite::SceneGraph::AmountOfScenes() const
{
	return m_pScenes.size();
}

size_t Elite::SceneGraph::Size() const
{
	return m_Objects.size();
}

#pragma endregion