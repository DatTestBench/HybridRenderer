#include "pch.h"
#include "Materials/Texture.hpp"
#include <SDL_image.h>

Texture::Texture(ID3D11Device* pDevice, const std::string& filePath)
	: m_pSurface(IMG_Load(filePath.c_str()))
	, m_pTexture(nullptr)
	, m_pTextureResourceView(nullptr)

{
	LoadTexture(pDevice, m_pSurface);
}

Texture::~Texture()
{
	if (m_pTextureResourceView)
		m_pTextureResourceView->Release();
	if (m_pTexture)
		m_pTexture->Release();
	if (m_pSurface != nullptr)
		SDL_FreeSurface(m_pSurface);
}

#pragma region Software

RGBColor Texture::Sample(const glm::vec2& uv) const
{
	glm::vec2 remappedUV;

	remappedUV.x = glm::clamp(uv.x, 0.f, 1.f) * m_pSurface->w;
	remappedUV.y = glm::clamp(uv.y, 0.f, 1.f) * m_pSurface->h;
	SDL_Color color;

	SDL_GetRGB(GetPixel(m_pSurface, static_cast<uint32_t>(remappedUV.x), static_cast<uint32_t>(remappedUV.y)), m_pSurface->format, &color.r, &color.g, &color.b);

	return { color.r / 255.f, color.g / 255.f, color.b / 255.f };
}
glm::vec4 Texture::Sample4(const glm::vec2& uv) const
{
	glm::vec2 remappedUV;

	remappedUV.x = glm::clamp(uv.x, 0.f, 1.f) * m_pSurface->w;
	remappedUV.y = glm::clamp(uv.y, 0.f, 1.f) * m_pSurface->h;
	SDL_Color color;

	SDL_GetRGBA(GetPixel(m_pSurface, static_cast<uint32_t>(remappedUV.x), static_cast<uint32_t>(remappedUV.y)), m_pSurface->format, &color.r, &color.g, &color.b, &color.a);

	return { color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f };
}

glm::vec3 Texture::SampleV(const glm::vec2& uv) const
{
	glm::vec2 remappedUV;

	remappedUV.x = glm::clamp(uv.x, 0.f, 1.f) * m_pSurface->w;
	remappedUV.y = glm::clamp(uv.y, 0.f, 1.f) * m_pSurface->h;
	SDL_Color color;

	SDL_GetRGB(GetPixel(m_pSurface, static_cast<int32_t>(remappedUV.x), static_cast<int32_t>(remappedUV.y)), m_pSurface->format, &color.r, &color.g, &color.b);

	return { color.r, color.g, color.b };
}
float Texture::SampleF(const glm::vec2& uv, const int32_t component) const
{
	switch (component)
	{
	case 0:
		return Sample(uv).r;
	case 1:
		return Sample(uv).g;
	case 2:
		return Sample(uv).b;
	default:
		return Sample(uv).r;
	}
}

// GetPixel function adapted from http://sdl.beuc.net/sdl.wiki/Pixel_Access
uint32_t Texture::GetPixel(SDL_Surface* surface, const uint32_t x, const uint32_t y)
{
	const int32_t bpp = surface->format->BytesPerPixel;

	auto* p = static_cast<uint8_t*>(surface->pixels) + y * surface->pitch + x * bpp;

	switch (bpp) {
	case 1:
		return *p;
	case 2:
		return *reinterpret_cast<uint16_t*>(p);
	case 3:
		if constexpr (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
		else
			return p[0] | p[1] << 8 | p[2] << 16;
	case 4:
		return *reinterpret_cast<uint32_t*>(p);
	default:
		return 0;
	}
}
#pragma endregion

#pragma region D3D
void Texture::LoadTexture(ID3D11Device* pDevice, SDL_Surface* pSurface)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = pSurface->w;
	desc.Height = pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(m_pTexture, &srvDesc, &m_pTextureResourceView);
}

#pragma endregion

