#include "pch.h"
#include "Effect.h"
#include "Texture.h"

namespace dae
{
	Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFilePath)
	{
		m_pEffect = Effect::LoadEffect(pDevice, assetFilePath);

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

		//Vertex layout
		static constexpr uint32_t numElements{ 4 };
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

		//Input layout
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

		m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
		if (!m_pNormalMapVariable->IsValid())
		{
			std::wcout << L"m_pNormalMapVariable is not valid\n";
		}

		m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
		if (!m_pSpecularMapVariable->IsValid())
		{
			std::wcout << L"m_pSpecularMapVariable is not valid\n";
		}

		m_pGlossyMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
		if (!m_pGlossyMapVariable->IsValid())
		{
			std::wcout << L"m_pGlossinessMapVariable is not valid\n";
		}

		m_pLightDirVariable = m_pEffect->GetVariableByName("gLightDir")->AsVector();
		if (!m_pLightDirVariable->IsValid())
		{
			std::wcout << L"m_pLightDirVariable is not valid\n";
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
	}

	Effect::~Effect()
	{
		m_pTechnique->Release();
		m_pEffect->Release();
		m_pInputLayout->Release();
	}

	ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFilePath)
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
	ID3DX11Effect* Effect::GetEffect() const
	{
		return m_pEffect;
	}

	ID3DX11EffectTechnique* Effect::GetTechnique() const
	{
		return m_pTechnique;
	}

	ID3D11InputLayout* Effect::GetInputLayout() const
	{
		return m_pInputLayout;
	}

	ID3DX11EffectMatrixVariable* Effect::GetWorldViewProjectionMatrix() const
	{
		return m_pMatWorldViewProjVariable;
	}

	void Effect::SetDiffuseMap(Texture* pTexture)
	{
		if (m_pDiffuseMapVariable)
		{
			m_pDiffuseMapVariable->SetResource(pTexture->GetSRV());
		}
	}

	void Effect::SetNormalMap(Texture* pTexture)
	{
		if (m_pNormalMapVariable)
		{
			m_pNormalMapVariable->SetResource(pTexture->GetSRV());
		}
	}

	void Effect::SetSpecularMap(Texture* pTexture)
	{
		if (m_pSpecularMapVariable)
		{
			m_pSpecularMapVariable->SetResource(pTexture->GetSRV());
		}
	}

	void Effect::SetGlossyMap(Texture* pTexture)
	{
		if (m_pGlossyMapVariable)
		{
			m_pGlossyMapVariable->SetResource(pTexture->GetSRV());
		}
	}

	void Effect::SetLightDirection(Vector3& lightDirection)
	{
		if (m_pLightDirVariable)
		{
			m_pLightDirVariable->SetFloatVector((float*)&lightDirection);
		}
	}

	void Effect::SetWorldMatrix(Matrix& worldMatrix)
	{
		if (m_pWorldVariable)
		{
			m_pWorldVariable->SetMatrix((float*)&worldMatrix);
		}
	}

	void Effect::SetInverseViewMatrix(Matrix& inverseView)
	{
		if (m_pViewInverseVariable)
		{
			m_pViewInverseVariable->SetMatrix((float*)&inverseView);
		}
	}
}