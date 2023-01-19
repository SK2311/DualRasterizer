#pragma once

#include "Texture.h"
#include <vector>
#include "DataTypes.h"
#include <map>

namespace dae
{
	class MeshData
	{
	public:
		MeshData() = default;
		MeshData(const MeshData&) = delete;
		MeshData(MeshData&&) noexcept = delete;
		MeshData& operator=(const MeshData&) = delete;
		MeshData& operator=(MeshData&&) noexcept = delete;
		~MeshData()
		{
			for (auto& texture : m_pTextureMap)
			{
				delete texture.second;
			}
		};

		std::vector<Vertex_In> vertices{};
		std::vector<Vertex_Out> vertices_out{};
		std::vector<uint32_t> indices{};

		PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleStrip };

		Matrix worldMatrix{};
		Matrix transformMatrix{};
		Matrix scaleMatrix{};
		Matrix rotationMatrix{};
		float yawRotation{};

		std::map<std::string, Texture*> m_pTextureMap{};

		void AddRotationY(float yaw)
		{
			yawRotation += yaw;
			rotationMatrix = Matrix::CreateRotationY(yawRotation);

			// Update world matrix 
			UpdateWorldMatrix();
		};

		Texture* GetTexture(std::string textureName) const
		{
			return m_pTextureMap.at(textureName);
		}

	private:
		void UpdateWorldMatrix()
		{
			worldMatrix = scaleMatrix * rotationMatrix * transformMatrix;
		}
	};
}