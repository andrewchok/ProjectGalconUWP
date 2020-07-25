#pragma once

#include "Transform2D.h"
#include "MatrixHelper.h"

namespace DirectXGame
{
	class PopupSprite final
	{
	public:
		enum struct PopupMenu
		{
			Invalid = -1,

			Pause,
			BlueWins,
			RedWins,

			Max
		};

		PopupSprite(std::uint32_t spriteIndex, const DX::Transform2D& transform, PopupMenu popupMenu = PopupMenu::Invalid, const DirectX::XMFLOAT4X4& textureTransform = DX::MatrixHelper::Identity);

		std::uint32_t SpriteIndex() const;
		void SetSpriteIndex(const std::uint32_t spriteIndex);

		const DX::Transform2D& Transform() const;
		void SetTransform(const DX::Transform2D& transform);

		PopupMenu getPopupMenu() const;
		void setPopupMenu(const PopupMenu percent);

		const DirectX::XMFLOAT4X4& TextureTransform() const;
		void SetTextureTransform(const DirectX::XMFLOAT4X4& transform);

	private:
		DirectX::XMFLOAT4X4 mTextureTransform;
		DX::Transform2D mTransform;
		std::uint32_t mSpriteIndex;
		PopupMenu mPopupMenu;
	};
}

