#include "pch.h"
#include "DirectXMesh.h"
#include "Effect.h"
#include "DataTypes.h"
#include "Mesh.h"
#include "FireEffect.h"

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

	DirectXMesh::DirectXMesh(ID3D11Device* pDevice, FireEffect* pEffect, MeshData* pMeshData, ID3D11DeviceContext* pDeviceContext)
		: m_pFireEffect{pEffect}
		, m_pDeviceContext{ pDeviceContext }
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
		delete m_pFireEffect;
	}

	void DirectXMesh::Render(Matrix& worldViewProjection) const
	{
		if (m_pFireEffect == nullptr)
		{
			//Render vehicle

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
		else
		{
			//Render thruster
			// 
			//1 Set primitive topology
			m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			//2 Set the input layout
			m_pDeviceContext->IASetInputLayout(m_pFireEffect->GetInputLayout());

			//3 Set VertexBuffer
			constexpr UINT stride = sizeof(Vertex_In);
			constexpr UINT offset = 0;

			m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
			m_pFireEffect->GetWorldViewProjectionMatrix()->SetMatrix((float*)(&worldViewProjection));

			//4 Set indexbuffer
			m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

			//5 Draw
			D3DX11_TECHNIQUE_DESC techDesc{};
			m_pFireEffect->GetTechnique()->GetDesc(&techDesc);
			for (UINT p = 0; p < techDesc.Passes; ++p)
			{
				m_pFireEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, m_pDeviceContext);
				m_pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
			}
		}
		
	}

	void DirectXMesh::SetLightDirection(Vector3& lightDir)
	{
		if (m_pFireEffect == nullptr)
		{
			m_pEffect->SetLightDirection(lightDir);
		}
		else
		{
			m_pFireEffect->SetLightDirection(lightDir);
		}
	}

	void DirectXMesh::SetWorldMatrix(Matrix& world)
	{
		if (m_pFireEffect == nullptr)
		{
			m_pEffect->SetWorldMatrix(world);
		}
		else
		{
			m_pFireEffect->SetWorldMatrix(world);
		}
	}

	void DirectXMesh::SetViewInverse(Matrix& viewInverse)
	{
		if (m_pFireEffect == nullptr)
		{
			m_pEffect->SetInverseViewMatrix(viewInverse);
		}
		else
		{
			m_pFireEffect->SetInverseViewMatrix(viewInverse);
		}
	}

	MeshData* DirectXMesh::GetMesh() const
	{
		return m_pMeshData;
	}

	Effect* DirectXMesh::GetEffect() const
	{
		return m_pEffect;
	}

	FireEffect* DirectXMesh::GetFireEffect() const
	{
		return m_pFireEffect;
	}
}