#include "pch.h"
#include "FireEffect.h"
#include "Texture.h"

namespace dae
{
	FireEffect::FireEffect(ID3D11Device* pDevice, const std::wstring& assetFilePath)
	{
		m_pEffect = FireEffect::LoadEffect(pDevice, assetFilePath);

		SetVariables();

		// Vertex layout
		const uint32_t numElements{ 4 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

		// Position
		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		// UV coordinates
		vertexDesc[1].SemanticName = "TEXCOORD";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		// Normal coordinates
		vertexDesc[2].SemanticName = "NORMAL";
		vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		// Tangent coordinates
		vertexDesc[3].SemanticName = "TANGENT";
		vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		// Create input layout
		D3DX11_PASS_DESC passDesc{};
		m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

		HRESULT result = pDevice->CreateInputLayout(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&m_pInputLayout
		);

		if (FAILED(result))
		{
			return;
		}
	}

	FireEffect::~FireEffect()
	{
		m_pTechnique->Release();
		m_pEffect->Release();
		m_pInputLayout->Release();
	}

	ID3DX11Effect* FireEffect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFilePath)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // defined(DEBUG) || defined(_DEBUG)

		result = D3DX11CompileEffectFromFile(
			assetFilePath.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

				std::wstringstream ss;
				for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); ++i)
				{
					ss << pErrors[i];
				}

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << '\n';
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFilePath;
				std::wcout << ss.str() << "\n";
				return nullptr;
			}
		}
		return pEffect;
	}

	ID3DX11Effect* FireEffect::GetEffect() const
	{
		return m_pEffect;
	}

	ID3DX11EffectTechnique* FireEffect::GetTechnique() const
	{
		return m_pTechnique;
	}

	ID3D11InputLayout* FireEffect::GetInputLayout() const
	{
		return m_pInputLayout;
	}

	ID3DX11EffectMatrixVariable* FireEffect::GetWorldViewProjectionMatrix() const
	{
		return m_pMatWorldViewProjVariable;
	}

	void FireEffect::SetDiffuseMap(Texture* pTexture)
	{
		if (m_pDiffuseMapVariable)
		{
			m_pDiffuseMapVariable->SetResource(pTexture->GetSRV());
		}
	}

	void FireEffect::SetWorldMatrix(Matrix& worldMatrix)
	{
		if (m_pWorldVariable)
		{
			m_pWorldVariable->SetMatrix((float*)&worldMatrix);
		}
	}

	void FireEffect::SetInverseViewMatrix(Matrix& inverseView)
	{
		if (m_pViewInverseVariable)
		{
			m_pViewInverseVariable->SetMatrix((float*)&inverseView);
		}
	}

	void FireEffect::SetVariables()
	{
		m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
		if (!m_pTechnique->IsValid())
		{
			std::wcout << L"Technique not valid\n";
		}

		m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pMatWorldViewProjVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldViewProjVariable not valid\n";
		}

		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVariable not valid\n";
		}

		m_pViewInverseVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
		if (!m_pViewInverseVariable->IsValid())
		{
			std::wcout << L"m_pViewInverseVariable is not valid\n";
		}

		m_pWorldVariable = m_pEffect->GetVariableByName("gWorld")->AsMatrix();
		if (!m_pWorldVariable->IsValid())
		{
			std::wcout << L"m_pWorldVariable is not valid\n";
		}

		m_pRasterizerState = m_pEffect->GetVariableByName("gRasterizerState")->AsRasterizer();
		if (!m_pRasterizerState->IsValid())
		{
			std::wcout << L"m_pRasterizerState is not valid\n";
		}
	}
}