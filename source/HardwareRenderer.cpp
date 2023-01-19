#include "pch.h"
#include "HardwareRenderer.h"
#include "Mesh.h"
#include "Effect.h"
#include "DirectXMesh.h"

namespace dae 
{
	HardwareRenderer::HardwareRenderer(SDL_Window* pWindow, Camera* pCamera, int width, int height, std::vector<MeshData*> pMeshes)
		: m_pWindow{pWindow}
		, m_pCamera{pCamera}
		, m_Width{width}
		, m_Height{height}
	{
		const HRESULT result{ InitializeDirectX() };

		Effect* vehicleEffect{ new Effect(m_pDevice, std::wstring{L"Resources/PosCol3D.fx"}) };
		m_pMeshes.push_back(new DirectXMesh(m_pDevice, vehicleEffect, pMeshes[0], m_pDeviceContext));

		for (auto pMesh : pMeshes)
		{
			for (auto texture : pMesh->m_pTextureMap)
			{
				if (texture.second == nullptr)
				{
					std::cout << texture.first << " was a nullptr\n";
					continue;
				}

				auto pSurface = texture.second->GetSurface();

				DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
				D3D11_TEXTURE2D_DESC desc{};
				desc.Width = pSurface->w;
				desc.Height = pSurface->h;
				desc.MipLevels = 1;
				desc.ArraySize = 1;
				desc.Format = format;
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

				auto pResource{ texture.second->GetResource() };
				HRESULT hr = m_pDevice->CreateTexture2D(&desc, &initData, &pResource);

				if (FAILED(hr))
				{
					throw std::runtime_error("Failed to create texture for direct3d");
				}

				texture.second->SetResource(pResource);

				D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
				SRVDesc.Format = format;
				SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				SRVDesc.Texture2D.MipLevels = 1;

				auto pResourceView{ texture.second->GetSRV() };

				hr = m_pDevice->CreateShaderResourceView(pResource, &SRVDesc, &pResourceView);

				if (FAILED(hr))
				{
					throw std::runtime_error("Failed to create resource view for texture");
				}

				texture.second->SetResourceView(pResourceView);
			}
		}

		for (auto pMesh : m_pMeshes)
		{
			pMesh->GetEffect()->SetDiffuseMap(pMesh->GetMesh()->GetTexture("DiffuseMap"));
			pMesh->GetEffect()->SetNormalMap(pMesh->GetMesh()->GetTexture("NormalMap"));
			pMesh->GetEffect()->SetSpecularMap(pMesh->GetMesh()->GetTexture("SpecularMap"));
			pMesh->GetEffect()->SetGlossyMap(pMesh->GetMesh()->GetTexture("GlossyMap"));
		}

		auto lightDir = Vector3{ 0.577f, -0.577f, 0.577f };
		for (auto pMesh : m_pMeshes)
		{
			pMesh->SetLightDirection(lightDir);
		}
	}

	HardwareRenderer::~HardwareRenderer()
	{
		if (m_pDeviceContext)
		{
			m_pRenderTargetView->Release();
			m_pRenderTargetBuffer->Release();

			m_pDepthStencilView->Release();
			m_pDepthStencilBuffer->Release();

			m_pSwapChain->Release();

			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();

			m_pDevice->Release();
		}

		for (auto pMesh : m_pMeshes)
		{
			delete pMesh;
		}
	}

	void HardwareRenderer::Update(const Timer* pTimer, bool shouldRotate)
	{
		m_pCamera->Update(pTimer);

		if (shouldRotate)
		{
			for (auto pMesh : m_pMeshes) 
			{
				const float degPerSec{ 25.0f };
				pMesh->GetMesh()->AddRotationY((degPerSec * pTimer->GetElapsed()) * TO_RADIANS);
			}
		}
	}

	void HardwareRenderer::Render() const
	{
		//1 Clear RTV & DSV
		ColorRGB clearColour = ColorRGB{ 0.0f,0.0f,0.3f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColour.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		Matrix worldViewProjection = m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix();

		//2 Set pipeline + draw calls
		for (const auto& pMesh : m_pMeshes)
		{
			auto mesh{ pMesh->GetMesh() };

			Matrix inverseView = m_pCamera->GetViewInverseMatrix();
			Matrix worldMatrix = mesh->worldMatrix;
			Matrix projectionMatrix = m_pCamera->GetProjectionMatrix();
			Matrix worldViewProjection = worldMatrix * m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix();

			pMesh->SetWorldMatrix(worldMatrix);
			pMesh->SetViewInverse(inverseView);
			pMesh->Render(worldViewProjection);
		}

		//3 Present backbuffer (swap)
		m_pSwapChain->Present(0, 0);
	}

	HRESULT HardwareRenderer::InitializeDirectX()
	{
		//1 Create device & device context
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // defined(DEBUG) || defined(_DEBUG)

		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel,
			1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);

		if (FAILED(result))
			return result;

		//Create DXGI factory
		IDXGIFactory1* pDxgiFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));

		if (FAILED(result))
			return result;

		//2 Create swapchain
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handle (HWND) from the SDL backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create actual swapchain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);

		if (FAILED(result))
			return result;

		pDxgiFactory->Release();

		//3 Create DepthStencil & DepthStencilView
		//Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);

		if (FAILED(result))
			return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);

		if (FAILED(result))
			return result;

		//4 Create RenderTarget & RenderTargetView
		//Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));

		if (FAILED(result))
			return result;

		//View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);

		if (FAILED(result))
			return result;

		//5 Bind RTV & DSV to output merger stage
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//6 Set viewport
		D3D11_VIEWPORT viewport{};
		viewport.Width = m_Width;
		viewport.Height = m_Height;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		return result;
	}
}