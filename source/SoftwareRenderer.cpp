#include "pch.h"
#include "SoftwareRenderer.h"
#include "SDL_surface.h"
#include "Mesh.h"
#include "Texture.h"

namespace dae
{
	SoftwareRenderer::SoftwareRenderer(SDL_Window* pWindow, Camera* pCamera, int width, int height, std::vector<MeshData*>& pMeshes, std::map<std::string, Texture*>& pTextureMap)
		: m_pWindow{pWindow}
		, m_pCamera{pCamera}
		, m_Width{width}
		, m_Height{height}
		, m_pMeshes{pMeshes}
		, m_pTextureMap{pTextureMap}
	{
		m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

		m_pDepthBufferPixels = new float[m_Width * m_Height];

		m_pCamera->Initialize((float)m_Width / (float)m_Height, 90.f, { 0.0f,0.0f,-10.f });


	}

	SoftwareRenderer::~SoftwareRenderer()
	{
		delete[] m_pDepthBufferPixels;

		for (auto& pMesh : m_pMeshes)
		{
			delete pMesh;
		}
	}

	void SoftwareRenderer::Update(const Timer* pTimer, bool shouldRotate, ShadingMode shadingMode)
	{
		m_pCamera->Update(pTimer);

		m_ShadingMode = shadingMode;

		if (shouldRotate)
		{
			for (auto pMesh : m_pMeshes)
			{
				const float degPerSec{ 25.0f };
				pMesh->AddRotationY((degPerSec * pTimer->GetElapsed()) * TO_RADIANS);
			}
		}		
	}

	void SoftwareRenderer::Render() const
	{
		SDL_LockSurface(m_pBackBuffer);

		VertexTransformationFunction();

		std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
		SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

		for (auto& pMesh : m_pMeshes)
		{
			//Change how the for loop advances based on the primitive topology
			int size = 0;
			std::vector<Vertex_Out> transformedVertices{ pMesh->vertices_out };

			if (pMesh->primitiveTopology == PrimitiveTopology::TriangleList)
			{
				size = (int)pMesh->indices.size();
			}
			else if (pMesh->primitiveTopology == PrimitiveTopology::TriangleStrip)
			{
				size = (int)pMesh->indices.size() - 2;
			}

			for (int i{}; i < size;)
			{
				int evenIndex{};
				if (pMesh->primitiveTopology == PrimitiveTopology::TriangleStrip)
				{
					evenIndex = i % 2;
				}

				int index0{ (int)pMesh->indices[i] };
				int index1{ (int)pMesh->indices[i + 1 + evenIndex] };
				int index2{ (int)pMesh->indices[i + 2 - evenIndex] };

				//Increase i based on primitiveTopology
				if (pMesh->primitiveTopology == PrimitiveTopology::TriangleList)
				{
					i += 3;
				}
				else if (pMesh->primitiveTopology == PrimitiveTopology::TriangleStrip)
				{
					++i;
				}

				//Calculate the points of the triangle
				Vector4 v0{ transformedVertices[index0].position };
				Vector4 v1{ transformedVertices[index1].position };
				Vector4 v2{ transformedVertices[index2].position };

				//Frustum Culling
				if (v0.x < -1.0f || v0.x > 1.0f || v0.y < -1.0f || v0.y > 1.0f || v0.z < 0.0f || v0.z > 1.0f ||
					v1.x < -1.0f || v1.x > 1.0f || v1.y < -1.0f || v1.y > 1.0f || v1.z < 0.0f || v1.z > 1.0f ||
					v2.x < -1.0f || v2.x > 1.0f || v2.y < -1.0f || v2.y > 1.0f || v2.z < 0.0f || v2.z > 1.0f)
				{
					continue;
				}

				//Pre-calculate value for the depth buffer -> depth buffer will not be linear anymore
				float v0InvDepth{ 1 / v0.w };
				float v1InvDepth{ 1 / v1.w };
				float v2InvDepth{ 1 / v2.w };

				//Convert from NDC to raster space
				//Go from [-1,1] range to [0,1] range, taking screen size into acount
				v0.x = ((v0.x + 1) / 2.0f) * m_Width;
				v0.y = ((1 - v0.y) / 2.0f) * m_Height;

				v1.x = ((v1.x + 1) / 2.0f) * m_Width;
				v1.y = ((1 - v1.y) / 2.0f) * m_Height;

				v2.x = ((v2.x + 1) / 2.0f) * m_Width;
				v2.y = ((1 - v2.y) / 2.0f) * m_Height;

				//Calculate the bounding box
				float xMin = std::min(std::min(v0.x, v1.x), v2.x);
				float xMax = std::max(std::max(v0.x, v1.x), v2.x);

				float yMin = std::min(std::min(v0.y, v1.y), v2.y);
				float yMax = std::max(std::max(v0.y, v1.y), v2.y);

				for (int py{ (int)yMin }; py < yMax; ++py)
				{
					for (int px{ (int)xMin }; px < xMax; ++px)
					{
						ColorRGB finalColor{ 0.f, 0.f, 0.f };

						//Current pixel
						Vector2 pixel{ (float)px,(float)py };

						//Check if the current pixel overlaps the triangle formed by the vertices
					//2D cross product gives a float, based on sign we know if the point is inside the triangle
						Vector2 edge0{ {v1.x - v0.x}, {v1.y - v0.y} };
						Vector2 pointToEdge0{ Vector2{v0.x, v0.y }, pixel };
						float cross0{ Vector2::Cross(edge0, pointToEdge0) };

						Vector2 edge1{ {v2.x - v1.x}, {v2.y - v1.y} };
						Vector2 pointToEdge1{ Vector2{v1.x, v1.y }, pixel };
						float cross1{ Vector2::Cross(edge1, pointToEdge1) };

						Vector2 edge2{ {v0.x - v2.x}, {v0.y - v2.y} };
						Vector2 pointToEdge2{ Vector2{v2.x, v2.y }, pixel };
						float cross2{ Vector2::Cross(edge2, pointToEdge2) };

						if (cross0 > 0.0f && cross1 > 0.0f && cross2 > 0.0f)
						{
							//Calculate the barycentric coordinates
							//2D cross product of V1V0 and V2V0
							float areaOfparallelogram{ Vector2::Cross(edge0, edge1) };

							//Calculate the weights
							float w0{ Vector2::Cross(edge1, pointToEdge1) / areaOfparallelogram };
							float w1{ Vector2::Cross(edge2, pointToEdge2) / areaOfparallelogram };
							float w2{ Vector2::Cross(edge0, pointToEdge0) / areaOfparallelogram };

							if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f)
							{
								//Do the depth buffer test
								float zBuffer0{ (1.0f / v0.z) * w0 };
								float zBuffer1{ (1.0f / v1.z) * w1 };
								float zBuffer2{ (1.0f / v2.z) * w2 };

								float zBuffer{ zBuffer0 + zBuffer1 + zBuffer2 };
								float invZBuffer{ 1.0f / zBuffer };

								if (invZBuffer < 0.0f || invZBuffer > 1.0f)
								{
									break;
								}

								if (invZBuffer < m_pDepthBufferPixels[px + (py * m_Width)])
								{
									//Write value of invZbuffer to the depthBuffer
									m_pDepthBufferPixels[px + (py * m_Width)] = invZBuffer;

									//Interpolated the depth value
									float wInterpolated{ 1.0f / ((w0 / v0.w) + (w1 / v1.w) + (w2 / v2.w)) };

									//Interpolated colour
									ColorRGB interpolatedColour{ transformedVertices[index0].color * (w0 / v0.w) +
																transformedVertices[index1].color * (w1 / v1.w) +
																transformedVertices[index2].color * (w2 / v2.w) };
									interpolatedColour *= wInterpolated;



									//Interpolated uv
									Vector2 interpolatedUV{ transformedVertices[index0].uv * (w0 / v0.w) +
															transformedVertices[index1].uv * (w1 / v1.w) +
															transformedVertices[index2].uv * (w2 / v2.w) };
									interpolatedUV *= wInterpolated;



									//Interpolated normal
									Vector3 interpolatedNormal{ transformedVertices[index0].normal * (w0 / v0.w) +
																transformedVertices[index1].normal * (w1 / v1.w) +
																transformedVertices[index2].normal * (w2 / v2.w) };
									interpolatedNormal *= wInterpolated;
									//Normalize direction vectors!
									interpolatedNormal.Normalize();



									//Interpolated tangent
									Vector3 interpolatedTangent{ transformedVertices[index0].tangent * (w0 / v0.w) +
																transformedVertices[index1].tangent * (w1 / v1.w) +
																transformedVertices[index2].tangent * (w2 / v2.w) };
									interpolatedTangent *= wInterpolated;
									//Normalize direction vectors!
									interpolatedTangent.Normalize();



									//Interpolated viewDirection
									Vector3 interpolatedViewDirection{ transformedVertices[index0].viewDirection * (w0 / v0.w) +
																		transformedVertices[index1].viewDirection * (w1 / v1.w) +
																		transformedVertices[index2].viewDirection * (w2 / v2.w) };
									interpolatedViewDirection *= wInterpolated;
									//Normalize direction vectors!
									interpolatedViewDirection.Normalize();


									Vertex_Out pixelInfo{};
									pixelInfo.position = Vector4{ pixel.x, pixel.y, invZBuffer, wInterpolated };
									pixelInfo.uv = interpolatedUV;
									pixelInfo.normal = interpolatedNormal;
									pixelInfo.tangent = interpolatedTangent;
									pixelInfo.viewDirection = interpolatedViewDirection;


									//Render the pixel
									finalColor = ShadePixel(pixelInfo);

									//Update Color in Buffer
									finalColor.MaxToOne();

									m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
										static_cast<uint8_t>(finalColor.r * 255),
										static_cast<uint8_t>(finalColor.g * 255),
										static_cast<uint8_t>(finalColor.b * 255));
								}
							}
						}
					}
				}
			}
		}

		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}

	void SoftwareRenderer::VertexTransformationFunction() const
	{
		Matrix worldViewProjectionMatrix{};

		for (auto& pMesh : m_pMeshes)
		{
			worldViewProjectionMatrix *= pMesh->worldMatrix * m_pCamera->viewMatrix * m_pCamera->projectionMatrix;
			pMesh->vertices_out.clear();

			for (auto& vertex : pMesh->vertices)
			{
				//Transfrom vertex from model space to screen space (aka raster space)
				Vector4 position{ Vector4{vertex.position, 1} };
				Vector4 transformedVertex{ worldViewProjectionMatrix.TransformPoint(position) };

				//Get the viewDirection from the vertex position
				Vector3 viewDirection{ pMesh->worldMatrix.TransformPoint(vertex.position) - m_pCamera->origin };

				//Do the perspective divide with the w component from the Vector4 transfromedVertex
				transformedVertex.x /= transformedVertex.w;
				transformedVertex.y /= transformedVertex.w;
				transformedVertex.z /= transformedVertex.w;

				//Normal and tangent info from vertex
				Vector3 normal = pMesh->worldMatrix.TransformVector(vertex.normal);
				normal.Normalize();
				Vector3 tangent = pMesh->worldMatrix.TransformVector(vertex.tangent);
				tangent.Normalize();

				Vertex_Out outVertex{};
				outVertex.color = vertex.color;
				outVertex.normal = normal;
				outVertex.position = transformedVertex;
				outVertex.tangent = tangent;
				outVertex.uv = vertex.uv;
				outVertex.viewDirection = viewDirection;

				pMesh->vertices_out.push_back(outVertex);
			}
		}
	}
	ColorRGB SoftwareRenderer::ShadePixel(const Vertex_Out& vertexOut) const
	{
		ColorRGB finalColour{};

		Vector3 lightDirection{ 0.577f,-0.577f,0.577f };
		float lightIntensity{ 7.0f };
		ColorRGB totalLight{ ColorRGB{1.0f,1.0f,1.0f} *lightIntensity };
		float shininess{ 25.0f };
		ColorRGB ambient{ 0.025f,0.025f,0.025f };

		//Diffuse map
		ColorRGB diffuse{ m_pTextureMap.at("DiffuseMap")->Sample(vertexOut.uv) };

		//Normal map
		Vector3 biNormal{ Vector3::Cross(vertexOut.normal, vertexOut.tangent).Normalized() };
		Matrix tangentAxisSpace{ Matrix{vertexOut.tangent, biNormal, vertexOut.normal, {0,0,0}} };

		ColorRGB normalColour{ m_pTextureMap.at("NormalMap")->Sample(vertexOut.uv)};
		Vector3 normal{ 2.0f * normalColour.r - 1.0f, 2.0f * normalColour.g - 1.0f, 2.0f * normalColour.b - 1.0f };
		normal = tangentAxisSpace.TransformVector(normal);
		normal.Normalize();

		//Glossy map
		ColorRGB gloss{ m_pTextureMap.at("GlossyMap")->Sample(vertexOut.uv) };

		//Specular map
		ColorRGB specular{ m_pTextureMap.at("SpecularMap")->Sample(vertexOut.uv) };

		//Calculate labert cosine
		//Make sure that the normal and the lightDirection point in the same direction (originally opposed to each other)
		float lambertCosine{ Vector3::Dot(normal,-lightDirection) };
		if (lambertCosine <= 0.0f)
		{
			finalColour = { 0.f,0.f,0.f };
			return finalColour;
		}

		switch (m_ShadingMode)
		{
		case ShadingMode::Combined:
		{
			ColorRGB phongExponent{ gloss * shininess };

			Vector3 reflect{ Vector3::Reflect(-lightDirection, normal) };
			float cosAlpha{ std::max(0.0f, Vector3::Dot(reflect, vertexOut.viewDirection)) };
			ColorRGB phong{ specular * std::powf(cosAlpha, phongExponent.r) };

			ColorRGB rho{ diffuse };
			ColorRGB diffuseColour{ rho / PI };

			finalColour = lambertCosine * totalLight * diffuseColour + (phong + ambient);
		}
		break;
		case ShadingMode::Diffuse:
		{
			ColorRGB rho{ diffuse };
			ColorRGB diffuseColour{ rho / PI };
			finalColour = totalLight * diffuseColour * lambertCosine;
		}
		break;
		case ShadingMode::Specular:
		{
			ColorRGB phongExponent{ gloss * shininess };

			Vector3 reflect{ Vector3::Reflect(-lightDirection, normal) };
			float cosAlpha{ std::max(0.0f, Vector3::Dot(reflect, vertexOut.viewDirection)) };
			ColorRGB phong{ specular * std::powf(cosAlpha, phongExponent.r) };

			finalColour = totalLight * phong * lambertCosine;
		}
		break;
		case ShadingMode::ObservedArea:
		{
			finalColour = { lambertCosine,lambertCosine,lambertCosine };
		}
		break;
		default:
			break;
		}

		return finalColour;
	}
}