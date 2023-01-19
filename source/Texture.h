#pragma once
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

namespace dae
{
	struct Vector2;

	class Texture
	{
	public:
		~Texture();

		static Texture* LoadFromFile(const std::string& path);
		ColorRGB Sample(const Vector2& uv) const;
		SDL_Surface* GetSurface() const;

		ID3D11ShaderResourceView* GetSRV() const;
		ID3D11Texture2D* GetResource() const;
		void SetResource(ID3D11Texture2D* pResource);
		void SetResourceView(ID3D11ShaderResourceView* pResourceView);

	private:
		Texture(SDL_Surface* pSurface);

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };

		ID3D11Texture2D* m_pResource{};
		ID3D11ShaderResourceView* m_pResourceView{};
	};
}