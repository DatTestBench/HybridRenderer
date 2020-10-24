#include "pch.h"

//Project includes
#include "Rendering/Renderer.hpp"

#pragma warning (push, 0)
#include "ImGui/imgui_impl_sdl.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_opengl2.h"
#pragma warning (pop)

#include "Materials/MaterialManager.hpp"
#include "Materials/MaterialMapped.hpp"
#include "Materials/MaterialFlat.hpp"
#include "Rendering/Camera.hpp"

Renderer::Renderer(SDL_Window* pWindow)
	: m_pWindow{ pWindow }
	, m_Width{}
	, m_Height{}
	, m_IsInitialized{ false }
	, m_pSceneGraph{ SceneGraph::GetInstance() }
{
	//Initialize
	/*General*/
	int width, height = 0;
	SDL_GetWindowSize(pWindow, &width, &height);
	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);
	m_pSceneGraph->SetCamera(glm::vec3(0, 5, 65), m_Width, m_Height, 60.f);
	// ELITE_OLD m_pSceneGraph->SetCamera(Elite::FPoint3(0, 5, 65), m_Width, m_Height, 60.f);
	
	/*Software*/
	
	const auto gl_context = SDL_GL_CreateContext(m_pWindow);
	SDL_GL_MakeCurrent(m_pWindow, gl_context);
	SDL_GL_SetSwapInterval(1); // Enable vsync
	
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = static_cast<uint32_t*>(m_pBackBuffer->pixels);
	m_pDepthBuffer = new float[static_cast<uint64_t>(m_Width * m_Height)];

	ImGui_ImplSDL2_InitForOpenGL(m_pWindow, gl_context);
	ImGui_ImplOpenGL2_Init();
	
	/*D3D*/
	HRESULT result = InitializeDirectX();

	if (FAILED(result))
	{
		m_IsInitialized = false;
		std::cout << "DirectX initialization failed\n";
	}
	else
	{
		m_IsInitialized = true;
		std::cout << "DirectX is ready\n";
	}

	ImGui_ImplSDL2_InitForD3D(m_pWindow);
	ImGui_ImplDX11_Init(m_pDevice, m_pDeviceContext);

	//Objects and materials are initialized here as m_pDevice is needed for object initialization
	MaterialManager::GetInstance()->AddMaterial(new MaterialMapped(m_pDevice, L"./Resources/Shaders/PosCol3D.fx", "./Resources/Textures/vehicle_diffuse.png", "./Resources/Textures/vehicle_normal.png", "./Resources/Textures/vehicle_gloss.png", "./Resources/Textures/vehicle_specular.png", 25.f, 1, false));
	MaterialManager::GetInstance()->AddMaterial(new MaterialFlat(m_pDevice, L"./Resources/Shaders/FlatTransparancy.fx", "./Resources/Textures/fireFX_diffuse.png", 2, true));
	m_pSceneGraph->AddScene(0);
	m_pSceneGraph->AddObjectToGraph(new Mesh(m_pDevice, "./Resources/Meshes/vehicle.obj", MaterialManager::GetInstance()->GetMaterial(1), glm::vec3(0, 0, 0)), 0);
	// ELITE_OLD m_pSceneGraph->AddObjectToGraph(new Mesh(m_pDevice, "./Resources/Meshes/vehicle.obj", MaterialManager::GetInstance()->GetMaterial(1), Elite::FPoint3(0, 0, 0)), 0);
	m_pSceneGraph->AddObjectToGraph(new Mesh(m_pDevice, "./Resources/Meshes/fireFX.obj", MaterialManager::GetInstance()->GetMaterial(2), glm::vec3(0, 0, 0)), 0);
	// ELITE_OLD m_pSceneGraph->AddObjectToGraph(new Mesh(m_pDevice, "./Resources/Meshes/fireFX.obj", MaterialManager::GetInstance()->GetMaterial(2), Elite::FPoint3(0, 0, 0)), 0);
}

Renderer::~Renderer()
{
	SafeDelete(m_pDepthBuffer);

	if (m_pRenderTargetView)
		m_pRenderTargetView->Release();
	if (m_pRenderTargetBuffer)
		m_pRenderTargetBuffer->Release();
	if (m_pDepthStencilView)
		m_pDepthStencilView->Release();
	if (m_pDepthStencilBuffer)
		m_pDepthStencilBuffer->Release();
	if (m_pSwapChain)
		m_pSwapChain->Release();
	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}

#if defined(DEBUG) || defined(_DEBUG)
	//Resource leak debugging http://seanmiddleditch.com/direct3d-11-debug-api-tricks/
	ID3D11Debug* pDebug;
	HRESULT hr = m_pDevice->QueryInterface(IID_PPV_ARGS(&pDebug));

	// dump output only if we actually grabbed a debug interface
	if (pDebug != nullptr)
	{
		pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		pDebug->Release();
		pDebug = nullptr;
	}
	(void) hr;
#endif
	if (m_pDevice)
		m_pDevice->Release();
	if (m_pDXGIFactory)
		m_pDXGIFactory->Release();


	ImGui_ImplDX11_Shutdown();
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplSDL2_Shutdown();

	SDL_GL_DeleteContext(SDL_GL_GetCurrentContext());
	
}

void Renderer::Render() const
{

	RGBColor clearColor{};
	// ELITE_OLD Elite::RGBColor clearColor{};
	switch (m_pSceneGraph->GetRenderSystem())
	{
	case RenderSystem::Software:
		{			
			clearColor = { 128.f, 128.f, 128.f };

			//glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			SDL_LockSurface(m_pBackBuffer);

			ImGui_ImplOpenGL2_NewFrame();
			ImGui_ImplSDL2_NewFrame(m_pWindow);
			ImGui::NewFrame();
		
			std::fill_n(m_pDepthBuffer, static_cast<uint64_t>(m_Width * m_Height), FLT_MAX);
			std::fill_n(static_cast<uint32_t*>(m_pBackBufferPixels), static_cast<uint64_t>(m_Width * m_Height), SDL_MapRGB(m_pBackBuffer->format,
                static_cast<uint8_t>(clearColor.r),
                static_cast<uint8_t>(clearColor.g),
                static_cast<uint8_t>(clearColor.b)));

		

			ImGui::ShowDemoWindow();
			for (auto& o : m_pSceneGraph->GetCurrentSceneObjects())
			{
				m_pSceneGraph->GetCamera()->MakeScreenSpace(o);
				o->Rasterize(m_pBackBuffer, static_cast<uint32_t*>(m_pBackBuffer->pixels), m_pDepthBuffer, m_Width, m_Height);
			}
			SDL_LockSurface(m_pBackBuffer);
			
			ImGui::Render();

			GLuint texture;
			
			glGenTextures( 1, &texture );
			glBindTexture( GL_TEXTURE_2D, texture );

			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_pBackBuffer->w, m_pBackBuffer->h, 0, GL_BGRA,GL_UNSIGNED_BYTE, m_pBackBuffer->pixels );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			
			glEnable(GL_TEXTURE_2D);
			{
				glBegin(GL_QUADS);
				{

					const auto uvLeft = 0.0f;
					const auto uvRight = 1.0f;
					const auto uvTop = 0.0f;
					const auto uvBottom = 1.0f;
					
					glTexCoord2f(uvLeft, uvBottom); glVertex3f(-1.f, -1.f, -1.f);
					glTexCoord2f(uvLeft, uvTop); glVertex3f(-1.f, 1.f, -1.f);
					glTexCoord2f(uvRight, uvTop); glVertex3f(1.f, 1.f, -1.f);
					glTexCoord2f(uvRight, uvBottom); glVertex3f(1.f, -1.f, -1.f);
				}
				glEnd();
			}
			
			glDisable(GL_TEXTURE_2D);
			glDeleteTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, 0);
		
			ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
			
			SDL_GL_SwapWindow(m_pWindow);
			break;
		}
	case D3D:
		if (!m_IsInitialized)
			return;

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplSDL2_NewFrame(m_pWindow);
		ImGui::NewFrame();
		
		
		
		//Clear Buffers
		clearColor = RGBColor(0.f, 0.f, 0.3f);
		// ELITE_OLD clearColor = Elite::RGBColor(0.f, 0.f, 0.3f);
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		//Render
		ImGui::ShowDemoWindow();
		for (auto& mesh : m_pSceneGraph->GetCurrentSceneObjects())
		{
			mesh->Render(m_pDeviceContext, m_pSceneGraph->GetCamera());
		}

		ImGui::Render();	
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		
		//Present
		m_pSwapChain->Present(0, 0);
		break;
	default:
		break;
	}
}

void Renderer::SwapRenderSystem() const
{
	const auto newRenderSystem = m_pSceneGraph->GetRenderSystem();

	switch (newRenderSystem)
	{
	case RenderSystem::Software:
		{
			ImGui_ImplSDL2_InitForOpenGL(m_pWindow, SDL_GL_GetCurrentContext());
			ImGui_ImplOpenGL2_Init();

			ImGui_ImplDX11_Shutdown();

			
		}
	case RenderSystem::D3D:
		{
			ImGui_ImplSDL2_InitForD3D(m_pWindow);
			ImGui_ImplDX11_Init(m_pDevice, m_pDeviceContext);
			ImGui_ImplOpenGL2_Shutdown();
		}
	}
}

HRESULT Renderer::InitializeDirectX()
{
	//Create Device and Device context, using hardware acceleration
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, nullptr, 0, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext);
	if (FAILED(result))
		return result;

	//Create DXGI Factory to create SwapChain based on hardware
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDXGIFactory));
	if (FAILED(result))
		return result;

	//Create SwapChain Descriptor
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	//Get the handle HWND from the SDL backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	//Create SwapChain and hook it into the handle of the SDL window
	result = m_pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	if (FAILED(result))
		return result;

	//Create the Depth/Stencil Buffer and View
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
	if (FAILED(result))
		return result;

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
		return result;

	//Create the RenderTargetView
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result))
		return result;
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
	if (FAILED(result))
		return result;

	//Bind the Views to the Output Merger Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//Set the Viewport
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(m_Width);
	viewPort.Height = static_cast<float>(m_Height);
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	viewPort.MinDepth = 0.f;
	viewPort.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewPort);

	return S_OK;
}


