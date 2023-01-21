#pragma once
#include "DataTypes.h"

namespace dae
{
	class Texture;

	class FireEffect
	{
	public:
		FireEffect(ID3D11Device* pDevice, const std::wstring& assetFilePath);
		~FireEffect();

		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFilePath);

		ID3DX11Effect* GetEffect() const;
		ID3DX11EffectTechnique* GetTechnique() const;
		ID3D11InputLayout* GetInputLayout() const;
		ID3DX11EffectMatrixVariable* GetWorldViewProjectionMatrix() const;

		void SetDiffuseMap(Texture* pTexture);
		void SetWorldMatrix(Matrix& worldMatrix);
		void SetInverseViewMatrix(Matrix& inverseView);

	private:
		void SetVariables();

		ID3DX11Effect* m_pEffect{};
		ID3DX11EffectTechnique* m_pTechnique{};

		ID3D11InputLayout* m_pInputLayout{};

		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};

		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};

		ID3DX11EffectMatrixVariable* m_pWorldVariable{};
		ID3DX11EffectMatrixVariable* m_pViewInverseVariable{};

		ID3DX11EffectRasterizerVariable* m_pRasterizerState{};
	};
}