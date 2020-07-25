#pragma once

#include "Transform2D.h"
#include "MatrixHelper.h"

namespace DirectXGame
{
	class NumberSprite final
	{
	public:
		enum class Owner
		{
			Invalid = -1,

			Neutral,
			BluePlayer,
			RedPlayer,

			Max
		};

		NumberSprite(std::uint32_t spriteIndex, const DX::Transform2D& transform, Owner owner = Owner::Neutral, const DirectX::XMFLOAT4X4& textureTransform = DX::MatrixHelper::Identity);

		std::uint32_t SpriteIndex() const;
		void SetSpriteIndex(const std::uint32_t spriteIndex);

		const DX::Transform2D& Transform() const;
		void SetTransform(const DX::Transform2D& transform);

		Owner GetOwner() const;
		void SetOwner(const Owner owner);

		const DirectX::XMFLOAT4X4& TextureTransform() const;
		void SetTextureTransform(const DirectX::XMFLOAT4X4& transform);

	private:
		DirectX::XMFLOAT4X4 mTextureTransform;
		DX::Transform2D mTransform;
		Owner mOwner;
		std::uint32_t mSpriteIndex;
	};
}