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

private:
    /*General*/
    SDL_Window* m_pWindow;
    uint32_t m_Width;
    uint32_t m_Height;

    bool m_IsInitialized;

    SceneGraph* m_pSceneGraph;

    /*Software*/
    SDL_Surface* m_pFrontBuffer = nullptr;
    SDL_Surface* m_pBackBuffer = nullptr;
    uint32_t* m_pBackBufferPixels = nullptr;
    float* m_pDepthBuffer = nullptr;

    /*D3D*/
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pDeviceContext;
    IDXGIFactory* m_pDXGIFactory;
    IDXGISwapChain* m_pSwapChain;

    ID3D11Resource* m_pRenderTargetBuffer;
    ID3D11RenderTargetView* m_pRenderTargetView;
    ID3D11Texture2D* m_pDepthStencilBuffer;
    ID3D11DepthStencilView* m_pDepthStencilView;
    HRESULT InitializeDirectX();
};


#endif // !RENDERER_HPP
