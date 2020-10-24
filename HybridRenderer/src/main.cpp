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




void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

void PrintToolTip()
{
	std::cout << "Movement: Unreal-like viewport controls\n\n";
	std::cout << "Button controls:\n";
	std::cout << "\tI: Show help screen\n";
	std::cout << "\tR: Toggle between software and D3D rendering\n";
	std::cout << "\tF: Toggle between filter state (order: point, linear, anisotropic)\n";
	std::cout << "\tT: Toggle transparency on/off (only available in D3D)\n";
	std::cout << "\tM: Toggle rendertype between color/depth-buffer (only available in software)\n";
	std::cout << "\t1/2: Increase/decrease scene respectively (" << SceneGraph::GetInstance()->AmountOfScenes() << " scene(s) currently loaded)\n";
	std::cout << "\tY: Toggle object rotation on/off\n";
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
	const uint32_t width = 1280;
	const uint32_t height = 720;
	auto* pWindow = SDL_CreateWindow(
		"Hybrid Renderer",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width, height, SDL_WINDOW_OPENGL);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	
	auto pTimer{ std::make_unique<Timer>() };
	auto pRenderer{ std::make_unique<Renderer>(pWindow) };
	PrintToolTip();
	
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
				if (e.key.keysym.scancode == SDL_SCANCODE_I)
					PrintToolTip();
				if (e.key.keysym.scancode == SDL_SCANCODE_2)
					SceneGraph::GetInstance()->IncreaseScene();
				if (e.key.keysym.scancode == SDL_SCANCODE_1)
					SceneGraph::GetInstance()->DecreaseScene();
				if (e.key.keysym.scancode == SDL_SCANCODE_F)
					MaterialManager::GetInstance()->ChangeFilterType();
				if (e.key.keysym.scancode == SDL_SCANCODE_T)
					SceneGraph::GetInstance()->ToggleTransparency();
				if (e.key.keysym.scancode == SDL_SCANCODE_R)
				{
					SceneGraph::GetInstance()->ToggleRenderSystem();
					pRenderer->SetImGuiRenderSystem();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_M)
					SceneGraph::GetInstance()->ToggleRenderType();
				if (e.key.keysym.scancode == SDL_SCANCODE_Y)
					SceneGraph::GetInstance()->ToggleObjectRotation();
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
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "FPS: " << pTimer->GetFPS() << std::endl;
		}

	}
	pTimer->Stop();

	//Shutdown "framework"
	SceneGraph::GetInstance()->Destroy();
	MaterialManager::GetInstance()->Destroy();
	pRenderer.reset(); // todo fix unique ptr setup, it's a smart pointer, so we shouldn't have to release it manually. One option might be destroying imgui context in the renderer itself?
	ImGui::DestroyContext();
	ShutDown(pWindow);
	return 0;
}