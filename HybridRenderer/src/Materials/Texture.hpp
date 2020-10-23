#ifndef TEXTURE_HPP
#define TEXTURE_HPP

//General Includes
#include <SDL.h>
#include <string>

//Project includes
#include "Helpers/RGBColor.hpp"
#include "Helpers/EMath.h"

class Texture
{
public:
    Texture(ID3D11Device* pDevice, const std::string& filePath);
    ~Texture();
    DEL_ROF(Texture)

    /*Software*/
    Elite::RGBColor Sample(const Elite::FVector2& uv) const;
    Elite::FVector4 Sample4(const Elite::FVector2& uv) const;
    Elite::FVector3 SampleV(const Elite::FVector2& uv) const;
    float SampleF(const Elite::FVector2& uv, int component = 0) const;

    /*D3D*/
    [[nodiscard]] constexpr auto GetTextureView() const noexcept -> ID3D11ShaderResourceView* { return m_pTextureResourceView; }
private:

    /*Software*/
    SDL_Surface* m_pSurface;

    static uint32_t GetPixel(SDL_Surface* surface, uint32_t x, uint32_t y);

    /*D3D*/
    ID3D11Texture2D* m_pTexture;
    ID3D11ShaderResourceView* m_pTextureResourceView;

    void LoadTexture(ID3D11Device* pDevice, SDL_Surface* pSurface);
};

#endif // !ELITE_TEXTURE
