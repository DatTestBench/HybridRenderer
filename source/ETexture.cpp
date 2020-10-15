#include "pch.h"
#include "ETexture.h"
#include <SDL_image.h>

Elite::Texture::Texture(ID3D11Device* pDevice, const std::string& filePath)
	: m_pTexture{ nullptr }
	, m_pTextureResourceView{ nullptr }
	, m_pSurface{ IMG_Load(filePath.c_str()) }
{
	LoadTexture(pDevice, m_pSurface);
}

Elite::Texture::~Texture()
{
	if (m_pTextureResourceView)
		m_pTextureResourceView->Release();
	if (m_pTexture)
		m_pTexture->Release();
	if (m_pSurface != nullptr)
		SDL_FreeSurface(m_pSurface);
}

#pragma region Software

Elite::RGBColor Elite::Texture::Sample(const FVector2& uv) const
{
	FVector2 remappedUV;

	remappedUV.x = Clamp(uv.x, 0.f, 1.f) * m_pSurface->w;
	remappedUV.y = Clamp(uv.y, 0.f, 1.f) * m_pSurface->h;
	SDL_Color color;

	SDL_GetRGB(GetPixel(m_pSurface, int(remappedUV.x), int(remappedUV.y)), m_pSurface->format, &color.r, &color.g, &color.b);

	return RGBColor(color.r / 255.f, color.g / 255.f, color.b / 255.f);
}
Elite::FVector4 Elite::Texture::Sample4(const FVector2& uv) const
{
	FVector2 remappedUV;

	remappedUV.x = Clamp(uv.x, 0.f, 1.f) * m_pSurface->w;
	remappedUV.y = Clamp(uv.y, 0.f, 1.f) * m_pSurface->h;
	SDL_Color color;

	SDL_GetRGBA(GetPixel(m_pSurface, int(remappedUV.x), int(remappedUV.y)), m_pSurface->format, &color.r, &color.g, &color.b, &color.a);

	return FVector4(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
}

Elite::FVector3 Elite::Texture::SampleV(const FVector2& uv) const
{
	FVector2 remappedUV;

	remappedUV.x = Clamp(uv.x, 0.f, 1.f) * m_pSurface->w;
	remappedUV.y = Clamp(uv.y, 0.f, 1.f) * m_pSurface->h;
	SDL_Color color;

	SDL_GetRGB(GetPixel(m_pSurface, int(remappedUV.x), int(remappedUV.y)), m_pSurface->format, &color.r, &color.g, &color.b);

	return FVector3(color.r, color.g, color.b);
}
float Elite::Texture::SampleF(const FVector2& uv, int component) const
{
	switch (component)
	{
	case 0:
		return Sample(uv).r;
		break;

	case 1:
		return Sample(uv).g;
		break;
	case 2:
		return Sample(uv).b;
		break;
	default:
		return Sample(uv).r;
		break;
	}
}

// GetPixel function adapted from http://sdl.beuc.net/sdl.wiki/Pixel_Access
uint32_t Elite::Texture::GetPixel(SDL_Surface* surface, uint64_t x, uint64_t y) const
{
	int bpp = surface->format->BytesPerPixel;

	uint8_t* p = (uint8_t*)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp) {
	case 1:
		return *p;
		break;
	case 2:
		return *(uint16_t*)p;
		break;
	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
		else
			return p[0] | p[1] << 8 | p[2] << 16;
		break;
	case 4:
		return *(uint32_t*)p;
		break;
	default:
		return 0;
	}
}
#pragma endregion

#pragma region D3D
void Elite::Texture::LoadTexture(ID3D11Device* pDevice, SDL_Surface* pSurface)
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

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = desc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_pTextureResourceView);
}

#pragma endregion

