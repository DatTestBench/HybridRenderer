#include "pch.h"

//Project includes
#include "Rendering/Renderer.hpp"
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
	m_pSceneGraph->SetCamera(Elite::FPoint3(0, 5, 65), m_Width, m_Height, 60.f);

	/*Software*/
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = static_cast<uint32_t*>(m_pBackBuffer->pixels);
	m_pDepthBuffer = new float[static_cast<uint64_t>(m_Width * m_Height)];


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

	//Objects and materials are initialized here as m_pDevice is needed for object initialization
	MaterialManager::GetInstance()->AddMaterial(new MaterialMapped(m_pDevice, L"./Resources/Shaders/PosCol3D.fx", "./Resources/Textures/vehicle_diffuse.png", "./Resources/Textures/vehicle_normal.png", "./Resources/Textures/vehicle_gloss.png", "./Resources/Textures/vehicle_specular.png", 25.f, 1, false));
	MaterialManager::GetInstance()->AddMaterial(new MaterialFlat(m_pDevice, L"./Resources/Shaders/FlatTransparancy.fx", "./Resources/Textures/fireFX_diffuse.png", 2, true));
	m_pSceneGraph->AddScene(0);
	m_pSceneGraph->AddObjectToGraph(new Mesh(m_pDevice, "./Resources/Meshes/vehicle.obj", MaterialManager::GetInstance()->GetMaterial(1), Elite::FPoint3(0, 0, 0)), 0);
	m_pSceneGraph->AddObjectToGraph(new Mesh(m_pDevice, "./Resources/Meshes/fireFX.obj", MaterialManager::GetInstance()->GetMaterial(2), Elite::FPoint3(0, 0, 0)), 0);
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
#endif
	if (m_pDevice)
		m_pDevice->Release();
	if (m_pDXGIFactory)
		m_pDXGIFactory->Release();
}

void Renderer::Render() const
{

	Elite::RGBColor clearColor{};
	switch (m_pSceneGraph->GetRenderSystem())
	{
	case RenderSystem::Software:
		clearColor = { 128.f, 128.f, 128.f };
		SDL_LockSurface(m_pBackBuffer);

		std::fill_n(m_pDepthBuffer, static_cast<uint64_t>(m_Width * m_Height), FLT_MAX);
		std::fill_n(m_pBackBufferPixels, static_cast<uint64_t>(m_Width * m_Height), SDL_MapRGB(m_pBackBuffer->format,
            static_cast<uint8_t>(clearColor.r),
            static_cast<uint8_t>(clearColor.g),
            static_cast<uint8_t>(clearColor.b)));

		for (auto& o : m_pSceneGraph->GetCurrentSceneObjects())
		{
			m_pSceneGraph->GetCamera()->MakeScreenSpace(o);
			o->Rasterize(m_pBackBuffer, m_pBackBufferPixels, m_pDepthBuffer, m_Width, m_Height);
		}

		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, nullptr, m_pFrontBuffer, nullptr);
		SDL_UpdateWindowSurface(m_pWindow);
		break;
	case D3D:
		if (!m_IsInitialized)
			return;

		//Clear Buffers
		clearColor = Elite::RGBColor(0.f, 0.f, 0.3f);
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		//Render
		for (auto& mesh : m_pSceneGraph->GetCurrentSceneObjects())
		{
			mesh->Render(m_pDeviceContext, m_pSceneGraph->GetCamera());
		}

		//Present
		m_pSwapChain->Present(0, 0);
		break;
	default:
		break;
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


