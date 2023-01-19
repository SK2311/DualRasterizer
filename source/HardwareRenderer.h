#pragma once
#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class MeshData;
	class DirectXMesh;
	class Texture;

	class HardwareRenderer
	{
	public:
		HardwareRenderer(SDL_Window* pWindow, Camera* pCamera, int width, int height, std::vector<MeshData*> pMeshes);
		~HardwareRenderer();

		void Update(const Timer* pTimer, bool shouldRotate);
		void Render() const;

	private:

		HRESULT InitializeDirectX();

		SDL_Window* m_pWindow{};
		Camera* m_pCamera;

		ID3D11Device* m_pDevice{};
		ID3D11DeviceContext* m_pDeviceContext{};

		IDXGISwapChain* m_pSwapChain{};

		ID3D11Texture2D* m_pDepthStencilBuffer{};
		ID3D11DepthStencilView* m_pDepthStencilView{};

		ID3D11Texture2D* m_pRenderTargetBuffer{};
		ID3D11RenderTargetView* m_pRenderTargetView{};

		std::vector<DirectXMesh*> m_pMeshes{};

		int m_Width{};
		int m_Height{};
	};
}

