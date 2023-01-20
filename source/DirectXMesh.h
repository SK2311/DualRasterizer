#pragma once

namespace dae
{
	class MeshData;
	class Effect;
	class FireEffect;

	class DirectXMesh
	{
	public:
		DirectXMesh(ID3D11Device* pDevice, Effect* pEffect, MeshData* pMeshData, ID3D11DeviceContext* pDeviceContext);
		DirectXMesh(ID3D11Device* pDevice, FireEffect* pEffect, MeshData* pMeshData, ID3D11DeviceContext* pDeviceContext);
		~DirectXMesh();

		//Rest of the rule of 5
		DirectXMesh(const DirectXMesh&) = delete;
		DirectXMesh(DirectXMesh&&) noexcept = delete;
		DirectXMesh& operator=(const DirectXMesh&) = delete;
		DirectXMesh& operator=(DirectXMesh&&) noexcept = delete;

		void Render(Matrix& worldViewProjection) const;
		void SetLightDirection(Vector3& lightDir);
		void SetWorldMatrix(Matrix& world);
		void SetViewInverse(Matrix& viewInverse);

		MeshData* GetMesh() const;

		Effect* GetEffect() const;
		FireEffect* GetFireEffect() const;

	private:
		MeshData* m_pMeshData{};

		Effect* m_pEffect{};
		FireEffect* m_pFireEffect{};

		ID3D11Buffer* m_pVertexBuffer{};
		ID3D11Buffer* m_pIndexBuffer{};

		uint32_t m_NumIndices{};

		ID3D11DeviceContext* m_pDeviceContext{};
	};
}