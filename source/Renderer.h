#pragma once
#include "Camera.h"
#include "SoftwareRenderer.h"
#include "HardwareRenderer.h"
#include <map>
#include "Texture.h"


struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

		void ToggleFPS();
		bool ShouldPrintFPS() const;

		void ToggleRenderer();
		void ToggleShadingMode();
		void ToggleVehicleRotation();
		void ToggleDepthBufferVis();
		void ToggleFire();
		void ToggleUniformColor();

	private:
		void LoadVehicleOBJ();
		void LoadThrusterOBJ();
		
		ShadingMode m_ShadingMode{};

		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		bool m_PrintFPS{ false };

		bool m_UseSoftware{ true };
		bool m_ShouldRotate{ true };
		bool m_ShowDepthBuffer{ false };
		bool m_ShowFire{ true };
		bool m_UniformColor{ false };

		Camera* m_pCamera{};

		SoftwareRenderer* m_pSoftwareRenderer{};
		HardwareRenderer* m_pHardwareRenderer{};

		std::vector<MeshData*> m_pMeshes{};

		std::map<std::string, Texture*> m_pTextureMap{};

		//DIRECTX
		//HRESULT InitializeDirectX();
		//...
	};
}
