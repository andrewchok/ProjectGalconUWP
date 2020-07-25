#include "pch.h"
#include "PopupSprite.h"

using namespace DX;
using namespace DirectX;

namespace DirectXGame
{
	PopupSprite::PopupSprite(uint32_t spriteIndex, const Transform2D& transform, PopupMenu popupMenu, const XMFLOAT4X4& textureTransform) :
		mSpriteIndex(spriteIndex), mTransform(transform), mPopupMenu(popupMenu), mTextureTransform(textureTransform)
	{
	}

	uint32_t PopupSprite::SpriteIndex() const
	{
		return mSpriteIndex;
	}

	void PopupSprite::SetSpriteIndex(const uint32_t spriteIndex)
	{
		mSpriteIndex = spriteIndex;
	}

	const Transform2D& PopupSprite::Transform() const
	{
		return mTransform;
	}

	void PopupSprite::SetTransform(const Transform2D& transform)
	{
		mTransform = transform;
	}

	PopupSprite::PopupMenu PopupSprite::getPopupMenu() const
	{
		return mPopupMenu;
	}

	void PopupSprite::setPopupMenu(const PopupMenu popupMenu)
	{
		mPopupMenu = popupMenu;
	}

	const XMFLOAT4X4& PopupSprite::TextureTransform() const
	{
		return mTextureTransform;
	}

	void PopupSprite::SetTextureTransform(const XMFLOAT4X4& transform)
	{
		mTextureTransform = transform;
	}
}