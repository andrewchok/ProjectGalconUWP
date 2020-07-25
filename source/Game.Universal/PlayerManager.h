#pragma once
#include <stdlib.h>
#include <vector>
#include "BaseUtils.h"
#include "Player.h"

namespace DirectXGame
{
	class PlayerManager final : public DX::DrawableGameComponent
	{
	public:
		/* Constants */
		static const uchar8_t NUM_PLAYERS;
		static const DirectX::XMFLOAT4 PLAYER_ONE_COLOR;
		static const DirectX::XMFLOAT4 PLAYER_TWO_COLOR;
		static const float_t PLAYER_START_SHIPCOUNT;
		static const float_t HIGHLIGHT_OFFSET;


	public:
		static PlayerManager* CreateInstance();
		static PlayerManager* GetInstance() { return CreateInstance(); };
		~PlayerManager() {};

		PlayerManager(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<DX::Camera>& camera);

		std::shared_ptr<Field> ActiveField() const;
		void SetActiveField(const std::shared_ptr<Field>& field);

		virtual void CreateDeviceDependentResources() override;
		virtual void ReleaseDeviceDependentResources() override;
		virtual void Update(const DX::StepTimer& timer) override;
		virtual void Render(const DX::StepTimer& timer) override;

		void init();
		void shutdown();

		// Get the of Player of player ID
		Player* getPlayer(int32_t playerID);
		std::vector<Player*> GetPlayers();

		int getPlayerTravellingShips(int32_t  playerID);

		int32_t getPlayerPercentageSelect(int32_t playerID);

	private:
		static PlayerManager* sInstance;
		std::vector<Player*> mPlayers;
		
		// graphics stuff
		void InitializeLineVertices();
		void InitializeCircleVertices();
		void DrawCircle(const Player& player);
		void DrawLine(const Player& player);

		void DrawShips(const Fleet& fleet);
		void DrawShip(const Ship& ship);
		void DrawDestinationLines(const Ship& ship);
		void DrawDirectionLines(const Ship& ship);

		static const std::uint32_t CircleResolution;
		static const std::uint32_t LineCircleVertexCount;
		static const std::uint32_t SolidCircleVertexCount;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mLineVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mCircleVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerObject;
		bool mLoadingComplete;
		std::vector<std::shared_ptr<Planet>> mPlanets;
		std::shared_ptr<Field> mActiveField;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
		std::uint32_t mIndexCount;
	};
}


