#ifndef ELITE_TEXTURE
#define ELITE_TEXTURE

//Project includes
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include "ERGBColor.h"
#include "EMath.h"

namespace Elite
{
		class Texture
		{
		public:
			Texture(ID3D11Device* pDevice, const std::string& filePath);
			~Texture();
			Texture(const Texture&) = delete;
			Texture& operator= (const Texture&) = delete;
			Texture(Texture&&) = delete;
			Texture& operator= (Texture&&) = delete;

			/*Software*/
			RGBColor Sample(const FVector2& uv) const;
			FVector4 Sample4(const FVector2& uv) const;
			FVector3 SampleV(const FVector2& uv) const;
			float SampleF(const FVector2& uv, int component = 0) const;

			/*D3D*/
			ID3D11ShaderResourceView* GetTextureView() const { return m_pTextureResourceView; }
		private:

			/*Software*/
			SDL_Surface* m_pSurface;

			static uint32_t GetPixel(SDL_Surface* surface, uint64_t x, uint64_t y);
			
			/*D3D*/
			ID3D11Texture2D* m_pTexture;
			ID3D11ShaderResourceView* m_pTextureResourceView;

			void LoadTexture(ID3D11Device* pDevice, SDL_Surface* pSurface);
		};
}


#endif // !ELITE_TEXTURE