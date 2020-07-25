#pragma once

#include "DrawableGameComponent.h"
#include "MatrixHelper.h"
#include <vector>
#include <random>

namespace DirectXGame
{
	class Sprite;

	class BackgroundRenderer final : public DX::DrawableGameComponent
	{
	public:
		BackgroundRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<DX::Camera>& camera);

		const DirectX::XMFLOAT2& Position() const;
		void SetPositon(const DirectX::XMFLOAT2& position);

		virtual void CreateDeviceDependentResources() override;
		virtual void ReleaseDeviceDependentResources() override;
		virtual void Update(const DX::StepTimer& timer) override;
		virtual void Render(const DX::StepTimer& timer) override;

		static const DirectX::XMFLOAT2 SpriteScale;

	private:
		struct VSCBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection;
			DirectX::XMFLOAT4X4 TextureTransform;

			VSCBufferPerObject() :
				WorldViewProjection(DX::MatrixHelper::Identity), TextureTransform(DX::MatrixHelper::Identity)
			{ };

			VSCBufferPerObject(const DirectX::XMFLOAT4X4& wvp, const DirectX::XMFLOAT4X4& textureTransform) :
				WorldViewProjection(wvp), TextureTransform(textureTransform)
			{ }
		};

		void DrawSprite(Sprite& sprite);
		void InitializeVertices();
		void InitializeSprites();

		static const DirectX::XMFLOAT2 UVScalingFactor;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSpriteSheet;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mTextureSampler;
		Microsoft::WRL::ComPtr<ID3D11BlendState> mAlphaBlending;
		VSCBufferPerObject mVSCBufferPerObjectData;
		bool mLoadingComplete;
		std::shared_ptr<Sprite> mSprite;
		std::uint32_t mIndexCount;
		std::uint32_t mSpriteRowCount;
		std::uint32_t mSpriteColumnCount;
		DirectX::XMFLOAT2 mPosition;
		//double mLastMoodUpdateTime;
		std::random_device mRandomDevice;
		std::default_random_engine mRandomGenerator;
	};
}

