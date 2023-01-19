#include "pch.h"
#include "DirectXMesh.h"
#include "Effect.h"
#include "DataTypes.h"
#include "Mesh.h"

namespace dae
{
	DirectXMesh::DirectXMesh(ID3D11Device* pDevice, Effect* pEffect, MeshData* pMeshData, ID3D11DeviceContext* pDeviceContext)
		: m_pEffect{pEffect}
		, m_pDeviceContext{pDeviceContext}
	{
		//Create vertex buffer
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(Vertex_In) * static_cast<uint32_t>(pMeshData->vertices.size());
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = pMeshData->vertices.data();

		HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);

		if (FAILED(result))
		{
			return;
		}

		//Create index buffer
		m_NumIndices = static_cast<uint32_t>(pMeshData->indices.size());
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		initData.pSysMem = pMeshData->indices.data();
		result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);

		if (FAILED(result))
		{
			return;
		}

		m_pMeshData = pMeshData;
	}

	DirectXMesh::~DirectXMesh()
	{
		m_pVertexBuffer->Release();
		m_pIndexBuffer->Release();

		delete m_pEffect;
	}

	void DirectXMesh::Render(Matrix& worldViewProjection) const
	{
		//1 Set primitive topology
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//2 Set the input layout
		m_pDeviceContext->IASetInputLayout(m_pEffect->GetInputLayout());

		//3 Set VertexBuffer
		constexpr UINT stride = sizeof(Vertex_In);
		constexpr UINT offset = 0;

		m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		m_pEffect->GetWorldViewProjectionMatrix()->SetMatrix((float*)(&worldViewProjection));

		//4 Set indexbuffer
		m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//5 Draw
		D3DX11_TECHNIQUE_DESC techDesc{};
		m_pEffect->GetTechnique()->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, m_pDeviceContext);
			m_pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
		}
	}

	void DirectXMesh::SetLightDirection(Vector3& lightDir)
	{
		m_pEffect->SetLightDirection(lightDir);
	}

	void DirectXMesh::SetWorldMatrix(Matrix& world)
	{
		m_pEffect->SetWorldMatrix(world);
	}

	void DirectXMesh::SetViewInverse(Matrix& viewInverse)
	{
		m_pEffect->SetInverseViewMatrix(viewInverse);
	}

	MeshData* DirectXMesh::GetMesh() const
	{
		return m_pMeshData;
	}

	Effect* DirectXMesh::GetEffect() const
	{
		return m_pEffect;
	}
}