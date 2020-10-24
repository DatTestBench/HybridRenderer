#pragma once

#include <iostream>
#include <algorithm>
#include <memory>
#define NOMINMAX  //for directx

// SDL Headers
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_surface.h>

#include <SDL_opengl.h>
#include <GL\GLU.h>
#include <SDL_image.h>

// DirectX Headers
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11effect.h>

#include <glm/glm.hpp>

//Elite headers
#include "Helpers/EMath.h"
#include "Helpers/RGBColor.hpp"

// Macros
#define DEL_ROF(className) \
className(const className&) = delete; \
className(className&&) noexcept = delete; \
className& operator= (const className&) = delete; \
className& operator= (className&&) noexcept = delete;

// Functions
template <class T>
inline void SafeDelete(T& pObject)
{
    if (pObject != nullptr)
    {
        delete (pObject);
        pObject = nullptr;
    }
}

template <class T>
inline void SafeDelete(T* pObject)
{
    if (pObject != nullptr)
    {
        delete (pObject);
        pObject = nullptr;
    }
}