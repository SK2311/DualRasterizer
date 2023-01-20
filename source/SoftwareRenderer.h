#pragma once
#include "Camera.h"
#include "DataTypes.h"
#include <map>

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class MeshData;
	class Texture;

	class SoftwareRenderer final
	{
	public:
		SoftwareRenderer(SDL_Window* pWindow, Camera* pCamera, int width, int height, std::vector<MeshData*>& pMeshes);
		~SoftwareRenderer();

		void Update(const Timer* pTimer, bool shouldRotate, ShadingMode shadingMode, bool showDepthBuffer, bool uniformColor);
		void Render() const;

	private:

		void VertexTransformationFunction() const;
		ColorRGB ShadePixel(const Vertex_Out& vertexOut) const;

		SDL_Window* m_pWindow{};
		Camera* m_pCamera{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		std::vector<MeshData*> m_pMeshes{};
		//std::map<std::string, Texture*> m_pTextureMap{};

		int m_Width{};
		int m_Height{};

		ShadingMode m_ShadingMode{};

		bool m_ShowDepthBuffer{};
		bool m_NormalMapEnabled{ true };
		bool m_UniformColor{};
	};
}