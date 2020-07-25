#include "pch.h"
#include "PercentSprite.h"

using namespace DX;
using namespace DirectX;

namespace DirectXGame
{
	PercentSprite::PercentSprite(uint32_t spriteIndex,  const Transform2D& transform, Percent percent, const XMFLOAT4X4& textureTransform) :
		mSpriteIndex(spriteIndex), mTransform(transform), mPercent(percent), mTextureTransform(textureTransform)
	{
	}

	uint32_t PercentSprite::SpriteIndex() const
	{
		return mSpriteIndex;
	}

	void PercentSprite::SetSpriteIndex(const uint32_t spriteIndex)
	{
		mSpriteIndex = spriteIndex;
	}

	const Transform2D& PercentSprite::Transform() const
	{
		return mTransform;
	}

	void PercentSprite::SetTransform(const Transform2D& transform)
	{
		mTransform = transform;
	}

	PercentSprite::Percent PercentSprite::getPercent() const
	{
		return mPercent;
	}

	void PercentSprite::setPercent(const Percent percent)
	{
		mPercent = percent;
	}

	const XMFLOAT4X4& PercentSprite::TextureTransform() const
	{
		return mTextureTransform;
	}

	void PercentSprite::SetTextureTransform(const XMFLOAT4X4& transform)
	{
		mTextureTransform = transform;
	}
}