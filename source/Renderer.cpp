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

		LoadVehicleOBJ();
		LoadThrusterOBJ();

		m_pCamera = new Camera();
		m_pCamera->Initialize((float)m_Width / (float)m_Height, 45.f, { 0.0f,0.0f,0.0f });

		m_pSoftwareRenderer = new SoftwareRenderer(m_pWindow, m_pCamera, m_Width, m_Height, m_pMeshes);
		m_pHardwareRenderer = new HardwareRenderer(m_pWindow, m_pCamera, m_Width, m_Height, m_pMeshes);

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
			m_pSoftwareRenderer->Update(pTimer, m_ShouldRotate, m_ShadingMode, m_ShowDepthBuffer, m_UniformColor, m_ShowBounding, m_RenderNormal, m_CullMode);
		}
		else
		{
			m_pHardwareRenderer->Update(pTimer, m_ShouldRotate, m_ShowFire, m_UniformColor, m_CullMode);
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
		std::cout << "\033[33m";
		m_PrintFPS ? std::cout << "**(SHARED) Print FPS ON" : std::cout << "**(SHARED) Print FPS OFF";
		std::cout << '\n';
	}

	bool Renderer::ShouldPrintFPS() const
	{
		return m_PrintFPS;
	}

	void Renderer::ToggleRenderer()
	{
		m_UseSoftware = !m_UseSoftware;
		std::cout << "\033[33m";
		m_UseSoftware ? std::cout << "**(SHARED) Rasterizer Mode = SOFTWARE" : std::cout << "**(SHARED) Rasterizer Mode = HARDWARE";
		std::cout << '\n';
	}

	void Renderer::ToggleShadingMode()
	{
		if (m_UseSoftware)
		{
			std::cout << "\033[35m";

			switch (m_ShadingMode)
			{
			case ShadingMode::Combined:
				m_ShadingMode = ShadingMode::ObservedArea;
				std::cout << "**(SOFTWARE) Shading Mode = OBSERVED_AREA";
				break;
			case ShadingMode::ObservedArea:
				m_ShadingMode = ShadingMode::Diffuse;
				std::cout << "**(SOFTWARE) Shading Mode = DIFFUSE";
				break;
			case ShadingMode::Diffuse:
				m_ShadingMode = ShadingMode::Specular;
				std::cout << "**(SOFTWARE) Shading Mode = SPECULAR";
				break;
			case ShadingMode::Specular:
				m_ShadingMode = ShadingMode::Combined;
				std::cout << "**(SOFTWARE) Shading Mode = COMBINED";
				break;
			default:
				break;
			}

			std::cout << '\n';
		}
	}

	void Renderer::ToggleVehicleRotation()
	{
		m_ShouldRotate = !m_ShouldRotate;
		std::cout << "\033[33m";
		m_ShouldRotate ? std::cout << "**(SHARED) Vehicle Rotation ON" : std::cout << "**(SHARED) Vehicle Rotation OFF";
		std::cout << '\n';
	}

	void Renderer::ToggleDepthBufferVis()
	{
		if (m_UseSoftware)
		{
			m_ShowDepthBuffer = !m_ShowDepthBuffer;
			std::cout << "\033[35m";
			m_ShowDepthBuffer ? std::cout << "**(SOFTWARE) DepthBuffer Visualization ON" : std::cout << "**(SOFTWARE) DepthBuffer Visualization OFF";
			std::cout << '\n';
		}
	}

	void Renderer::ToggleFire()
	{
		if (!m_UseSoftware)
		{
			m_ShowFire = !m_ShowFire;
			std::cout << "\033[32m";
			m_ShowFire ? std::cout << "**(HARDWARE) FireFX ON" : std::cout << "**(HARDWARE) FireFX OFF";
			std::cout << '\n';
		}
	}

	void Renderer::ToggleUniformColor()
	{
		m_UniformColor = !m_UniformColor;
		std::cout << "\033[33m";
		m_UniformColor ? std::cout << "**(SHARED) Uniform ClearColor ON" : std::cout << "**(SHARED) Uniform ClearColor OFF";
		std::cout << '\n';
	}

	void Renderer::ToggleBounding()
	{
		if (m_UseSoftware)
		{
			m_ShowBounding = !m_ShowBounding;
			std::cout << "\033[35m";
			m_ShowBounding ? std::cout << "**(SOFTWARE) BoundingBox Visualization ON" : std::cout << "**(SOFTWARE) BoundingBox Visualization OFF";
			std::cout << '\n';
		}
	}

	void Renderer::ToggleNormal()
	{
		if (m_UseSoftware)
		{
			m_RenderNormal = !m_RenderNormal;
			std::cout << "\033[35m";
			m_RenderNormal ? std::cout << "**(SOFTWARE) NormalMap ON" : std::cout << "**(SOFTWARE) NormalMap OFF";
			std::cout << '\n';
		}
	}

	void Renderer::ToggleSampleMode()
	{
		if (!m_UseSoftware)
		{
			m_pHardwareRenderer->ToggleSampleState();
		}
	}

	void Renderer::ToggleCulling()
	{
		std::cout << "\033[33m";

		switch (m_CullMode)
		{
		case dae::CullMode::BackFace:
			m_CullMode = CullMode::FrontFace;
			std::cout << "**(SHARED) CullMode = FRONT";
			break;
		case dae::CullMode::FrontFace:
			m_CullMode = CullMode::DoubleFace;
			std::cout << "**(SHARED) CullMode = NONE";
			break;
		case dae::CullMode::DoubleFace:
			m_CullMode = CullMode::BackFace;
			std::cout << "**(SHARED) CullMode = BACK";
			break;
		default:
			break;
		}

		std::cout << '\n';
	}

	void Renderer::LoadVehicleOBJ()
	{
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

		m_pMeshes.push_back(pMesh);
	}

	void Renderer::LoadThrusterOBJ()
	{
		std::vector<Vertex_In> vertices{};
		std::vector<uint32_t> indices{};
		Utils::ParseOBJ("Resources/fireFX.obj", vertices, indices);

		MeshData* pMesh{ new MeshData() };
		pMesh->vertices = vertices;
		pMesh->indices = indices;
		pMesh->transformMatrix = Matrix::CreateTranslation({ 0,0,50 });
		pMesh->scaleMatrix = Matrix::CreateScale({ 1,1,1 });
		pMesh->yawRotation = 90.f * TO_RADIANS;
		pMesh->rotationMatrix = Matrix::CreateRotationY(pMesh->yawRotation);
		pMesh->worldMatrix = pMesh->scaleMatrix * pMesh->rotationMatrix * pMesh->transformMatrix;
		pMesh->primitiveTopology = PrimitiveTopology::TriangleList;

		pMesh->m_pTextureMap.insert(std::make_pair("fireFX", Texture::LoadFromFile("Resources/fireFX_diffuse.png")));

		m_pMeshes.push_back(pMesh);
	}
}
