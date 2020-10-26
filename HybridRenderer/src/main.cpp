#include "pch.h"
#include "vld.h"
//#undef main

//Standard includes
#include <iostream>
#include <memory>

//Project includes
#include "Helpers/Timer.hpp"
#pragma warning (push, 0)
#include "ImGui/imgui_impl_sdl.h"
#pragma warning (pop)
#include "Materials/MaterialManager.hpp"
#include "Rendering/Renderer.hpp"
#include "Scene/SceneGraph.hpp"
#include "Rendering/Camera.hpp"
#include "Helpers/magic_enum.hpp"



void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

void ShowSettingsWindow() noexcept
{

}

int main(int argc, char* argv[])
{
	//Unreferenced parameters
	(void)argc;
	(void)argv;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	// OpenGL versions
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	const uint32_t width = 1920;
	const uint32_t height = 1080;
	auto* pWindow = SDL_CreateWindow(
		"Hybrid Renderer",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width, height, SDL_WINDOW_OPENGL);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	ImGui::CreateContext();
	
	auto pTimer = new Timer;
	const auto pRenderer = new Renderer(pWindow);
	SceneGraph::GetInstance()->SetTimer(pTimer);
	
	//Start loop
	pTimer->Start();
	auto printTimer = 0.f;
	auto isLooping = true;

	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			ImGui_ImplSDL2_ProcessEvent(&e);
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				break;
			default:
				break;
			}
		}

		//--------- Updates ---------
		SceneGraph::GetInstance()->GetCamera()->Update(pTimer->GetElapsed());
		SceneGraph::GetInstance()->Update(pTimer->GetElapsed());
		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();

	}
	pTimer->Stop();

	//Shutdown "framework"
	SceneGraph::GetInstance()->Destroy();
	MaterialManager::GetInstance()->Destroy();
	Logger::GetInstance()->Destroy();
	SafeDelete(pRenderer);
	SafeDelete(pTimer);
	ImGui::DestroyContext();
	ShutDown(pWindow);
	return 0;
}