#include"pch.h"
#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}

		m_pResource->Release();
		m_pResourceView->Release();
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		//TODO
		//Load SDL_Surface using IMG_LOAD
		//Create & Return a new Texture Object (using SDL_Surface)

		SDL_Surface* pSurface{ IMG_Load(path.c_str()) };
		return new Texture(pSurface);
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//TODO
		//Sample the correct texel for the given uv

		Uint8 r, g, b;

		const size_t x{ static_cast<size_t>(uv.x * m_pSurface->w) };
		const size_t y{ static_cast<size_t>(uv.y * m_pSurface->h) };

		const Uint32 pixel{ m_pSurfacePixels[x + y * m_pSurface->w] };

		SDL_GetRGB(pixel, m_pSurface->format, &r, &g, &b);

		const constexpr float invClampVal{ 1 / 255.f };

		return { r * invClampVal,g * invClampVal,b * invClampVal };
	}

	SDL_Surface* Texture::GetSurface() const
	{
		return m_pSurface;
	}

	ID3D11ShaderResourceView* Texture::GetSRV() const
	{
		return m_pResourceView;
	}

	ID3D11Texture2D* Texture::GetResource() const
	{
		return m_pResource;
	}

	void Texture::SetResource(ID3D11Texture2D* pResource)
	{
		m_pResource = pResource;
	}

	void Texture::SetResourceView(ID3D11ShaderResourceView* pResourceView)
	{
		m_pResourceView = pResourceView;
	}
}