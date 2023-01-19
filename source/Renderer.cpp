#include "pch.h"
#include "Renderer.h"
#include "DataTypes.h"
#include "Utils.h"
#include "Mesh.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
		, m_ShadingMode{ShadingMode::Combined}
	{
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		std::vector<Vertex_In> vertices{};
		std::vector<uint32_t> indices{};
		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);

		MeshData* pMesh{ new MeshData() };
		pMesh->vertices = vertices;
		pMesh->indices = indices;
		pMesh->transformMatrix = Matrix::CreateTranslation({ 0,0,50 });
		pMesh->scaleMatrix = Matrix::CreateScale({ 1,1,1 });
		pMesh->yawRotation = 90.f * TO_RADIANS;
		pMesh->rotationMatrix = Matrix::CreateRotationY(pMesh->yawRotation);
		pMesh->worldMatrix = pMesh->scaleMatrix * pMesh->rotationMatrix * pMesh->transformMatrix;
		pMesh->primitiveTopology = PrimitiveTopology::TriangleList;

		pMesh->m_pTextureMap.insert(std::make_pair("DiffuseMap", Texture::LoadFromFile("Resources/vehicle_diffuse.png")));
		pMesh->m_pTextureMap.insert(std::make_pair("NormalMap", Texture::LoadFromFile("Resources/vehicle_normal.png")));
		pMesh->m_pTextureMap.insert(std::make_pair("SpecularMap", Texture::LoadFromFile("Resources/vehicle_specular.png")));
		pMesh->m_pTextureMap.insert(std::make_pair("GlossyMap", Texture::LoadFromFile("Resources/vehicle_gloss.png")));

		std::vector<MeshData*> pMeshes{ pMesh };

		m_pCamera = new Camera();
		m_pCamera->Initialize((float)m_Width / (float)m_Height, 90.f, { 0.0f,0.0f,-10.f });

		m_pSoftwareRenderer = new SoftwareRenderer(m_pWindow, m_pCamera, m_Width, m_Height, pMeshes);
		m_pHardwareRenderer = new HardwareRenderer(m_pWindow, m_pCamera, m_Width, m_Height, pMeshes);

		std::cout << "\033[33m";
		std::cout << "[Key Bindings - SHARED]\n";
		std::cout << "\t[F1] Toggle Rasterizer Mode (HARDWARE/SOFTWARE)\n";
		std::cout << "\t[F2] Toggle Vehicle Rotation (ON/OFF)\n";
		std::cout << "\t[F9] Cycle CullMode (BACK/FRONT/NONE)\n";
		std::cout << "\t[F10] Toggle Uniform ClearColor (ON/OFF)\n";
		std::cout << "\t[F11] Toggle Print FPS (ON/OFF)\n";
		std::cout << '\n';

		std::cout << "\033[32m";
		std::cout << "[Key Bindings - HARDWARE]\n";
		std::cout << "\t[F3] Toggle FireFX (ON / OFF)\n";
		std::cout << "\t[F4] Cycle Sampler State (POINT / LINEAR / ANISOTROPIC)\n";
		std::cout << '\n';

		std::cout << "\033[35m";
		std::cout << "[Key Bindings - SOFTWARE]\n";
		std::cout << "\t[F5] Cycle Shading Mode (COMBINED / OBSERVED_AREA / DIFFUSE / SPECULAR)\n";
		std::cout << "\t[F6] Toggle NormalMap (ON / OFF\n";
		std::cout << "\t[F7] Toggle DepthBuffer Visualization (ON / OFF)\n";
		std::cout << "\t[F8] Toggle BoundingBox Visualization (ON / OFF)\n";
		std::cout << "\033[0m";
		std::cout << '\n';
		std::cout << '\n';
	}

	Renderer::~Renderer()
	{
		delete m_pHardwareRenderer;
		delete m_pSoftwareRenderer;
		delete m_pCamera;

		for (auto& texture : m_pTextureMap)
		{
			delete texture.second;
		}
	}

	void Renderer::Update(const Timer* pTimer)
	{
		if (m_UseSoftware)
		{
			m_pSoftwareRenderer->Update(pTimer, m_ShouldRotate, m_ShadingMode, m_ShowDepthBuffer);
		}
		else
		{
			m_pHardwareRenderer->Update(pTimer, m_ShouldRotate);
		}
	}

	void Renderer::Render() const
	{
		if (m_UseSoftware)
		{
			m_pSoftwareRenderer->Render();
		}
		else
		{
			m_pHardwareRenderer->Render();
		}
	}

	void Renderer::ToggleFPS()
	{
		m_PrintFPS = !m_PrintFPS;
	}

	bool Renderer::ShouldPrintFPS() const
	{
		return m_PrintFPS;
	}

	void Renderer::ToggleRenderer()
	{
		m_UseSoftware = !m_UseSoftware;
	}

	void Renderer::ToggleShadingMode()
	{
		if (!m_UseSoftware)
			return;

		switch (m_ShadingMode)
		{
		case ShadingMode::Combined:
			m_ShadingMode = ShadingMode::ObservedArea;
			break;
		case ShadingMode::ObservedArea:
			m_ShadingMode = ShadingMode::Diffuse;
			break;
		case ShadingMode::Diffuse:
			m_ShadingMode = ShadingMode::Specular;
			break;
		case ShadingMode::Specular:
			m_ShadingMode = ShadingMode::Combined;
			break;
		default:
			break;
		}
	}

	void Renderer::ToggleVehicleRotation()
	{
		m_ShouldRotate = !m_ShouldRotate;
	}

	void Renderer::ToggleDepthBufferVis()
	{
		m_ShowDepthBuffer = !m_ShowDepthBuffer;
	}

	/*HRESULT Renderer::InitializeDirectX()
	{
		return S_FALSE;
	}*/
}
