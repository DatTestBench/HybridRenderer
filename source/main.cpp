#include "pch.h"
#include "vld.h"
//#undef main

//Standard includes
#include <iostream>

//Project includes
#include "ETimer.h"
#include "ERenderer.h"
#include "ESceneGraph.h"
#include "ECamera.h"

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
	std::cout << "\tT: Toggle transparancy on/off (only available in D3D)\n";
	std::cout << "\tM: Toggle rendertype between color/depthbuffer (only available in software)\n";
	std::cout << "\t1/2: Increase/decrease scene respectively (" << Elite::SceneGraph::GetInstance()->AmountOfScenes() << " scene(s) currently loaded)\n";
	std::cout << "\tY: Toggle object rotation on/off\n";
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 1280;
	const uint32_t height = 720;
	SDL_Window* pWindow = SDL_CreateWindow(
		"DualRenderer - **Matthieu Limelette**",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	auto pTimer{ std::make_unique<Elite::Timer>() };
	auto pRenderer{ std::make_unique<Elite::Renderer>(pWindow) };
	PrintToolTip();
	
	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;

	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if (e.key.keysym.scancode == SDL_SCANCODE_I)
					PrintToolTip();
				if (e.key.keysym.scancode == SDL_SCANCODE_2)
					Elite::SceneGraph::GetInstance()->IncreaseScene();
				if (e.key.keysym.scancode == SDL_SCANCODE_1)
					Elite::SceneGraph::GetInstance()->DecreaseScene();
				if (e.key.keysym.scancode == SDL_SCANCODE_F)
					Elite::MaterialManager::GetInstance()->ChangeFilterType();
				if (e.key.keysym.scancode == SDL_SCANCODE_T)
					Elite::SceneGraph::GetInstance()->ToggleTransparancy();
				if (e.key.keysym.scancode == SDL_SCANCODE_R)
					Elite::SceneGraph::GetInstance()->ToggleRenderSystem();
				if (e.key.keysym.scancode == SDL_SCANCODE_M)
					Elite::SceneGraph::GetInstance()->ToggleRenderType();
				if (e.key.keysym.scancode == SDL_SCANCODE_Y)
					Elite::SceneGraph::GetInstance()->ToggleObjectRotation();
				break;
			}
		}

		//--------- Updates ---------
		Elite::SceneGraph::GetInstance()->GetCamera()->Update(pTimer->GetElapsed());
		Elite::SceneGraph::GetInstance()->Update(pTimer->GetElapsed());
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
	Elite::SceneGraph::GetInstance()->Destroy();
	Elite::MaterialManager::GetInstance()->Destroy();
	ShutDown(pWindow);
	return 0;
}