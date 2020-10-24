#ifndef RENDERER_HPP
#define	RENDERER_HPP

#include <cstdint>

// Project includes
#include "Scene/SceneGraph.hpp"

struct SDL_Window;
struct SDL_Surface;

class Renderer final
{
public:
    explicit Renderer(SDL_Window* pWindow);
    ~Renderer();

    DEL_ROF(Renderer)

    void Render() const;
    void SetImGuiRenderSystem(bool isInitialSetup = false) const;
private:
    /*General*/
    SDL_Window* m_pWindow;
    uint32_t m_Width;
    uint32_t m_Height;

    bool m_IsInitialized;

    SceneGraph* m_pSceneGraph;

    /*Software*/
    SDL_Surface* m_pSoftwareBuffer = nullptr;
    uint32_t* m_pSoftwareBufferPixels = nullptr;
    float* m_pDepthBuffer = nullptr;

    //Setup
    void SetupSoftwarePipeline() noexcept;
    void ImplementSoftwareWithOpenGL() const noexcept;
    
    /*D3D*/
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pDeviceContext;
    IDXGIFactory* m_pDXGIFactory;
    IDXGISwapChain* m_pSwapChain;

    ID3D11Resource* m_pRenderTargetBuffer;
    ID3D11RenderTargetView* m_pRenderTargetView;
    ID3D11Texture2D* m_pDepthStencilBuffer;
    ID3D11DepthStencilView* m_pDepthStencilView;


    void SetupDirectXPipeline() noexcept;
    HRESULT InitializeDirectX();
    void DirectXCleanup() const noexcept;
};


#endif // !RENDERER_HPP
