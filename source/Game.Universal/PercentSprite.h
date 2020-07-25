#pragma once

#include "Transform2D.h"
#include "MatrixHelper.h"

namespace DirectXGame
{
	class PercentSprite final
	{
	public:
		enum struct Percent
		{
			Invalid = -1,

			_25,
			_50,
			_75,
			_100,

			Max
		};

		PercentSprite(std::uint32_t spriteIndex, const DX::Transform2D& transform, Percent percent = Percent::_50, const DirectX::XMFLOAT4X4& textureTransform = DX::MatrixHelper::Identity);

		std::uint32_t SpriteIndex() const;
		void SetSpriteIndex(const std::uint32_t spriteIndex);

		const DX::Transform2D& Transform() const;
		void SetTransform(const DX::Transform2D& transform);

		Percent getPercent() const;
		void setPercent(const Percent percent);

		const DirectX::XMFLOAT4X4& TextureTransform() const;
		void SetTextureTransform(const DirectX::XMFLOAT4X4& transform);

	private:
		DirectX::XMFLOAT4X4 mTextureTransform;
		DX::Transform2D mTransform;
		std::uint32_t mSpriteIndex;
		Percent mPercent;
	};
}

