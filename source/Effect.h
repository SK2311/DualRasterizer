#pragma once

namespace dae
{
	class Texture;

	class Effect
	{
	public:
		Effect(ID3D11Device* pDevice, const std::wstring& assetFilePath);
		~Effect();

		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFilePath);

		ID3DX11Effect* GetEffect() const;
		ID3DX11EffectTechnique* GetTechnique() const;
		ID3D11InputLayout* GetInputLayout() const;
		ID3DX11EffectMatrixVariable* GetWorldViewProjectionMatrix() const;

		void SetDiffuseMap(Texture* pTexture);
		void SetNormalMap(Texture* pTexture);
		void SetSpecularMap(Texture* pTexture);
		void SetGlossyMap(Texture* pTexture);

		void SetLightDirection(Vector3& lightDirection);
		void SetWorldMatrix(Matrix& worldMatrix);
		void SetInverseViewMatrix(Matrix& inverseView);

	private:
		ID3DX11Effect* m_pEffect{};
		ID3DX11EffectTechnique* m_pTechnique{};

		ID3D11InputLayout* m_pInputLayout{};

		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};

		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pGlossyMapVariable{};

		ID3DX11EffectVectorVariable* m_pLightDirVariable{};

		ID3DX11EffectMatrixVariable* m_pWorldVariable{};
		ID3DX11EffectMatrixVariable* m_pViewInverseVariable{};
	};
}