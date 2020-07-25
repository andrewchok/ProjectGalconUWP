#include "pch.h"
#include "PercentSpriteManager.h"
#include "PercentSprite.h"
#include "StateManager.h"
#include "PlayerManager.h"

using namespace std;
using namespace DX;
using namespace DirectX;
using namespace Microsoft::WRL;

namespace DirectXGame
{
	//const DirectX::XMFLOAT2 PercentSpriteManager::SpriteScale = XMFLOAT2(800.0f, 450.0f);
	const DirectX::XMFLOAT2 PercentSpriteManager::SpriteScale = XMFLOAT2(1000.0f/4, 100.0f/4);
	const uint32_t PercentSpriteManager::SpriteCountCol = 2; // Sprites are arranged horizontally within the sprite sheet
	const uint32_t PercentSpriteManager::SpriteCountRow = 4; // Moods are arranged vertically within the sprite sheet
	const XMFLOAT2 PercentSpriteManager::UVScalingFactor = XMFLOAT2(1.0f / SpriteCountCol, 1.0f / SpriteCountRow);

	PercentSpriteManager::PercentSpriteManager(const shared_ptr<DX::DeviceResources>& deviceResources, const shared_ptr<Camera>& camera, uint32_t spriteRowCount, uint32_t spriteColumCount) :
		DrawableGameComponent(deviceResources, camera),
		mLoadingComplete(false), mIndexCount(0),
		mSpriteRowCount(spriteRowCount), mSpriteColumnCount(spriteColumCount),
		mPosition(0.0f, 0.0f)
	{
	}

	const XMFLOAT2& PercentSpriteManager::Position() const
	{
		return mPosition;
	}

	void PercentSpriteManager::SetPositon(const XMFLOAT2& position)
	{
		mPosition = position;
	}

	void PercentSpriteManager::CreateDeviceDependentResources()
	{
		auto loadVSTask = ReadDataAsync(L"SpriteRendererVS.cso");
		auto loadPSTask = ReadDataAsync(L"SpriteRendererPS.cso");

		// After the vertex shader file is loaded, create the shader and input layout.
		auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
			ThrowIfFailed(
				mDeviceResources->GetD3DDevice()->CreateVertexShader(
					&fileData[0],
					fileData.size(),
					nullptr,
					mVertexShader.ReleaseAndGetAddressOf()
				)
			);

			// Create an input layout
			ThrowIfFailed(
				mDeviceResources->GetD3DDevice()->CreateInputLayout(
					VertexPositionTexture::InputElements,
					VertexPositionTexture::InputElementCount,
					&fileData[0],
					fileData.size(),
					mInputLayout.ReleaseAndGetAddressOf()
				)
			);

			CD3D11_BUFFER_DESC constantBufferDesc(sizeof(VSCBufferPerObject), D3D11_BIND_CONSTANT_BUFFER);
			ThrowIfFailed(
				mDeviceResources->GetD3DDevice()->CreateBuffer(
					&constantBufferDesc,
					nullptr,
					mVSCBufferPerObject.ReleaseAndGetAddressOf()
				)
			);
		});

		// After the pixel shader file is loaded, create the shader and texture sampler state.
		auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
			ThrowIfFailed(
				mDeviceResources->GetD3DDevice()->CreatePixelShader(
					&fileData[0],
					fileData.size(),
					nullptr,
					mPixelShader.ReleaseAndGetAddressOf()
				)
			);

			D3D11_SAMPLER_DESC samplerStateDesc;
			ZeroMemory(&samplerStateDesc, sizeof(samplerStateDesc));
			samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerStateDesc.MinLOD = -FLT_MAX;
			samplerStateDesc.MaxLOD = FLT_MAX;
			samplerStateDesc.MipLODBias = 0.0f;
			samplerStateDesc.MaxAnisotropy = 1;
			samplerStateDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateSamplerState(&samplerStateDesc, mTextureSampler.ReleaseAndGetAddressOf()));

			D3D11_BLEND_DESC blendStateDesc = { 0 };
			blendStateDesc.RenderTarget[0].BlendEnable = true;
			blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBlendState(&blendStateDesc, mAlphaBlending.ReleaseAndGetAddressOf()));
		});

		auto loadSpriteSheetAndCreateSpritesTask = (createPSTask && createVSTask).then([this]() {
			ThrowIfFailed(CreateWICTextureFromFile(mDeviceResources->GetD3DDevice(), L"Content\\Textures\\PercentSpriteSheet.png", nullptr, mSpriteSheet.ReleaseAndGetAddressOf()));
			InitializeVertices();
			InitializeSprites();
		});

		// Once the cube is loaded, the object is ready to be rendered.
		loadSpriteSheetAndCreateSpritesTask.then([this]() {
			mLoadingComplete = true;
		});
	}

	void PercentSpriteManager::ReleaseDeviceDependentResources()
	{
		mLoadingComplete = false;
		mVertexShader.Reset();
		mPixelShader.Reset();
		mInputLayout.Reset();
		mVertexBuffer.Reset();
		mIndexBuffer.Reset();
		mVSCBufferPerObject.Reset();
		mSpriteSheet.Reset();
		mTextureSampler.Reset();
	}

	void PercentSpriteManager::Update(const StepTimer& timer)
	{
		UNREFERENCED_PARAMETER(timer);

		if (!mLoadingComplete)
		{
			return;
		}

		if (StateManager::GetInstance()->getState() == StateManager::State::RUNNING)
		{
			ChangeSprite(*(mSprites.at(0)), static_cast<PercentSprite::Percent>(PlayerManager::GetInstance()->getPlayer(0)->GetPecentageSelection()/25 -1));
			ChangeSprite(*(mSprites.at(1)), static_cast<PercentSprite::Percent>(PlayerManager::GetInstance()->getPlayer(1)->GetPecentageSelection()/25 -1));
		}
	}	

	void PercentSpriteManager::Render(const StepTimer& timer)
	{
		UNREFERENCED_PARAMETER(timer);

		if (StateManager::GetInstance()->getState() == StateManager::State::MENU ||
			StateManager::GetInstance()->getState() == StateManager::State::RESTART)
			return;

		// Loading is asynchronous. Only draw geometry after it's loaded.
		if (!mLoadingComplete)
		{
			return;
		}

		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		static const UINT stride = sizeof(VertexPositionTexture);
		static const UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, mVSCBufferPerObject.GetAddressOf());
		direct3DDeviceContext->PSSetShaderResources(0, 1, mSpriteSheet.GetAddressOf());
		direct3DDeviceContext->PSSetSamplers(0, 1, mTextureSampler.GetAddressOf());
		direct3DDeviceContext->OMSetBlendState(mAlphaBlending.Get(), 0, 0xFFFFFFFF);

		for (const auto& sprite : mSprites)
		{
			DrawSprite(*sprite);
		}
	}

	void PercentSpriteManager::DrawSprite(PercentSprite& sprite)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();

		const XMMATRIX wvp = XMMatrixTranspose(sprite.Transform().WorldMatrix() * mCamera->ViewProjectionMatrix());
		XMStoreFloat4x4(&mVSCBufferPerObjectData.WorldViewProjection, wvp);
		XMMATRIX textureTransform = XMLoadFloat4x4(&sprite.TextureTransform());
		XMStoreFloat4x4(&mVSCBufferPerObjectData.TextureTransform, XMMatrixTranspose(textureTransform));
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, &mVSCBufferPerObjectData, 0, 0);

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}

	void PercentSpriteManager::InitializeVertices()
	{
		VertexPositionTexture vertices[] =
		{
			VertexPositionTexture(XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)),
			VertexPositionTexture(XMFLOAT4(-1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)),
			VertexPositionTexture(XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)),
			VertexPositionTexture(XMFLOAT4(1.0f, -1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)),
		};

		D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
		vertexBufferDesc.ByteWidth = sizeof(VertexPositionTexture) * ARRAYSIZE(vertices);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData = { 0 };
		vertexSubResourceData.pSysMem = vertices;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mVertexBuffer.ReleaseAndGetAddressOf()));

		// Create and index buffer
		const uint32_t indices[] =
		{
			0, 1, 2,
			0, 2, 3
		};

		mIndexCount = ARRAYSIZE(indices);

		D3D11_BUFFER_DESC indexBufferDesc = { 0 };
		indexBufferDesc.ByteWidth = sizeof(uint32_t) * mIndexCount;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubResourceData = { 0 };
		indexSubResourceData.pSysMem = indices;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexSubResourceData, mIndexBuffer.ReleaseAndGetAddressOf()));
	}

	void PercentSpriteManager::InitializeSprites()
	{
		const XMFLOAT2 offset(1000.0f / 4, 100.0f / 4);
		{
			XMFLOAT2 position
			(
				mPosition.x * SpriteScale.x - OrthographicCamera::DefaultViewWidth / 2 + offset.x,
				mPosition.y * SpriteScale.y + OrthographicCamera::DefaultViewHeight / 2 - offset.y
			);
			Transform2D transform(position, 0.0f, SpriteScale);
			auto sprite = make_shared<PercentSprite>(0, transform);
			ChangeSprite(*sprite, PercentSprite::Percent::_50);
			mSprites.push_back(move(sprite));
		}

		{
			XMFLOAT2 position
			(
				mPosition.x * SpriteScale.x + OrthographicCamera::DefaultViewWidth / 2 - offset.x,
				mPosition.y * SpriteScale.y + OrthographicCamera::DefaultViewHeight / 2 - offset.y
			);
			Transform2D transform(position, 0.0f, SpriteScale);
			auto sprite = make_shared<PercentSprite>(1, transform);
			ChangeSprite(*sprite, PercentSprite::Percent::_50);
			mSprites.push_back(move(sprite));
		}
	}

	void PercentSpriteManager::ChangeSprite(PercentSprite& sprite, PercentSprite::Percent percent)
	{
		if (percent == PercentSprite::Percent::Invalid)
		{
			int32_t spritePercentIndex = static_cast<int>(sprite.getPercent()) + 1;
			if (spritePercentIndex > 3) spritePercentIndex = 0;
			sprite.setPercent(static_cast<PercentSprite::Percent>(spritePercentIndex));
		}
		else
		{
			sprite.setPercent(percent);
		}

		XMFLOAT4X4 textureTransform;
		XMMATRIX textureTransformMatrix = XMMatrixScaling(UVScalingFactor.x, UVScalingFactor.y, 0) * XMMatrixTranslation(UVScalingFactor.x * sprite.SpriteIndex(), UVScalingFactor.y * static_cast<int32_t>(sprite.getPercent()), 0.0f);
		XMStoreFloat4x4(&textureTransform, textureTransformMatrix);
		sprite.SetTextureTransform(textureTransform);
	}

}