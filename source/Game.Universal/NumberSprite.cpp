#include "pch.h"
#include "NumberSprite.h"

using namespace DX;
using namespace DirectX;

namespace DirectXGame
{
	NumberSprite::NumberSprite(uint32_t spriteIndex, const Transform2D& transform, Owner owner, const XMFLOAT4X4& textureTransform) :
		mSpriteIndex(spriteIndex), mTransform(transform), mOwner(owner), mTextureTransform(textureTransform)
	{
	}

	uint32_t NumberSprite::SpriteIndex() const
	{
		return mSpriteIndex;
	}

	void NumberSprite::SetSpriteIndex(const uint32_t spriteIndex)
	{
		mSpriteIndex = spriteIndex;
	}

	const Transform2D& NumberSprite::Transform() const
	{
		return mTransform;
	}

	void NumberSprite::SetTransform(const Transform2D& transform)
	{
		mTransform = transform;
	}

	NumberSprite::Owner NumberSprite::GetOwner() const
	{
		return mOwner;
	}

	void NumberSprite::SetOwner(const Owner owner)
	{
		mOwner = owner;
	}

	const XMFLOAT4X4& NumberSprite::TextureTransform() const
	{
		return mTextureTransform;
	}

	void NumberSprite::SetTextureTransform(const XMFLOAT4X4& transform)
	{
		mTextureTransform = transform;
	}
}