#include "pch.h"
#include "GameMain.h"
#include "StateManager.h"
#include "PlayerManager.h"
#include "PlanetManager.h"
#include "PopupSpriteManager.h"
#include "BackgroundRenderer.h"
#include "PercentSpriteManager.h"
#include "MenuRenderer.h"

using namespace DX;
using namespace std;
using namespace DirectX;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;
using namespace Concurrency;

namespace DirectXGame
{
	// Loads and initializes application assets when the application is loaded.
	GameMain::GameMain(const shared_ptr<DX::DeviceResources>& deviceResources) :
		mDeviceResources(deviceResources)
	{
		// Register to be notified if the Device is lost or recreated
		mDeviceResources->RegisterDeviceNotify(this);

		auto camera = make_shared<OrthographicCamera>(mDeviceResources);
		mComponents.push_back(camera);
		camera->SetPosition(0, 0, 1);

		CoreWindow^ window = CoreWindow::GetForCurrentThread();
		mKeyboard = make_shared<KeyboardComponent>(mDeviceResources);		
		mKeyboard->Keyboard()->SetWindow(window);
		mComponents.push_back(mKeyboard);

		mMouse = make_shared<MouseComponent>(mDeviceResources);		
		mMouse->Mouse()->SetWindow(window);
		mComponents.push_back(mMouse);

		mGamePad = make_shared<GamePadComponent>(mDeviceResources);
		mComponents.push_back(mGamePad);
		
		//auto fpsTextRenderer = make_shared<FpsTextRenderer>(mDeviceResources);
		//mComponents.push_back(fpsTextRenderer);

		auto menu = make_shared<MenuRenderer>(mDeviceResources, camera);
		mComponents.push_back(menu);

		//auto fieldManager = make_shared<FieldManager>(mDeviceResources, camera);
		//mComponents.push_back(fieldManager);

		//auto ballManager = make_shared<BallManager>(mDeviceResources, camera);
		//ballManager->SetActiveField(fieldManager->ActiveField());
		//mComponents.push_back(ballManager);

		//const int32_t spriteRowCount = 12;
		//const int32_t spriteColumnCount = 15;
		//auto spriteDemoManager = make_shared<SpriteDemoManager>(mDeviceResources, camera, spriteRowCount, spriteColumnCount);		
		//const XMFLOAT2 center((-spriteColumnCount + 1) * SpriteDemoManager::SpriteScale.x, (-spriteRowCount + 1) * SpriteDemoManager::SpriteScale.y);
		//spriteDemoManager->SetPositon(center);
		//mComponents.push_back(spriteDemoManager);

		//const int32_t spriteRowCount = 1;
		//const int32_t spriteColumnCount = 3;
		//auto numberSpriteManager = make_shared<NumberSpriteManager>(mDeviceResources, camera, spriteRowCount, spriteColumnCount);
		//const XMFLOAT2 center((-spriteColumnCount + 1) * NumberSpriteManager::SpriteScale.x, (-spriteRowCount + 1) * NumberSpriteManager::SpriteScale.y);
		//numberSpriteManager->SetPositon(center);
		//mComponents.push_back(numberSpriteManager);

		// add draw text on top of planets here

		auto popup = make_shared<PopupSpriteManager>(mDeviceResources, camera);
		mComponents.push_back(popup);

		auto percent = make_shared<PercentSpriteManager>(mDeviceResources, camera);
		mComponents.push_back(percent);

		auto planetManager = make_shared<PlanetManager>(mDeviceResources, camera);
		mComponents.push_back(planetManager);

		auto playerManager = make_shared<PlayerManager>(mDeviceResources, camera);
		mComponents.push_back(playerManager);

		auto background = make_shared<BackgroundRenderer>(mDeviceResources, camera);
		mComponents.push_back(background);

		mTimer.SetFixedTimeStep(true);
		mTimer.SetTargetElapsedSeconds(1.0 / 60);

		IntializeResources();
		init();
	}

	GameMain::~GameMain()
	{
		mDeviceResources->RegisterDeviceNotify(nullptr);
	}

	// Updates application state when the window size changes (e.g. device orientation change)
	void GameMain::CreateWindowSizeDependentResources()
	{
		for (auto& component : mComponents)
		{
			component->CreateWindowSizeDependentResources();
		}
	}

	// Updates the application state once per frame.
	void GameMain::Update()
	{
		// Update scene objects.
		mTimer.Tick([&]()
		{
			for (auto& component : mComponents)
			{
				component->Update(mTimer);
			}

			if (mKeyboard->WasKeyPressedThisFrame(Keys::Escape) ||
				mMouse->WasButtonPressedThisFrame(MouseButtons::Middle) ||
				mGamePad->WasButtonPressedThisFrame(GamePadButtons::Back)) 
			{
				CoreApplication::Exit();
			}

			// State check inputs
			{
				switch (StateManager::GetInstance()->getState())
				{
				case StateManager::State::MENU:
					if (mKeyboard->WasKeyPressedThisFrame(Keys::Space))
						StateManager::GetInstance()->setState(StateManager::State::RUNNING);
					break;

				case StateManager::State::RUNNING:
					if (mKeyboard->WasKeyPressedThisFrame(Keys::P))
						StateManager::GetInstance()->setState(StateManager::State::PAUSED);
					break;

				case StateManager::State::PAUSED:
					if (mKeyboard->WasKeyPressedThisFrame(Keys::P))
						StateManager::GetInstance()->setState(StateManager::State::RUNNING);
					break;

				case StateManager::State::GAME_OVER:
					if (mKeyboard->WasKeyPressedThisFrame(Keys::Space))
						StateManager::GetInstance()->setState(StateManager::State::RESTART);
					break;

				default:
					break;
				}

				if (mKeyboard->WasKeyPressedThisFrame(Keys::M))
				{
					if (StateManager::GetInstance()->getMode() == StateManager::Mode::DEBUG)
					{
						StateManager::GetInstance()->setMode(StateManager::Mode::GAME);
					}
					else
					{
						StateManager::GetInstance()->setMode(StateManager::Mode::DEBUG);
					}
				}

				if (StateManager::GetInstance()->getMode() == StateManager::Mode::DEBUG)
				{
					if (mKeyboard->WasKeyPressedThisFrame(Keys::O))
					{
						StateManager::GetInstance()->setState(StateManager::State::MENU);

						PlayerManager::GetInstance()->shutdown();
						PlanetManager::GetInstance()->deletePlanets();

						init();
					}
				}
			}
			
			// Player inputs
			if (StateManager::GetInstance()->getState() == StateManager::State::RUNNING)
			{
				for (int32_t i = 0; i < static_cast<int32_t>(PlayerManager::GetInstance()->GetPlayers().size()); i++)
				{
					if (i == 0)
					{ 
						if (mKeyboard->WasKeyPressedThisFrame(Keys::A))
							PlayerManager::GetInstance()->getPlayer(i)->setHighlightedPlanet(Planet::Neighbor::LEFT);
						else if (mKeyboard->WasKeyPressedThisFrame(Keys::D))
							PlayerManager::GetInstance()->getPlayer(i)->setHighlightedPlanet(Planet::Neighbor::RIGHT);
						else if (mKeyboard->WasKeyPressedThisFrame(Keys::W))
							PlayerManager::GetInstance()->getPlayer(i)->setHighlightedPlanet(Planet::Neighbor::UP);
						else if (mKeyboard->WasKeyPressedThisFrame(Keys::S))
							PlayerManager::GetInstance()->getPlayer(i)->setHighlightedPlanet(Planet::Neighbor::DOWN);

						if (mKeyboard->WasKeyPressedThisFrame(Keys::Q))
							PlayerManager::GetInstance()->getPlayer(i)->changePercentageSelection();

						if (mKeyboard->WasKeyPressedThisFrame(Keys::E)) 
							PlayerManager::GetInstance()->getPlayer(i)->selectPlanet();
					}

					if (i == 1)
					{
						if (mKeyboard->WasKeyPressedThisFrame(Keys::NumPad4))
							PlayerManager::GetInstance()->getPlayer(i)->setHighlightedPlanet(Planet::Neighbor::LEFT);
						else if (mKeyboard->WasKeyPressedThisFrame(Keys::NumPad6))
							PlayerManager::GetInstance()->getPlayer(i)->setHighlightedPlanet(Planet::Neighbor::RIGHT);
						else if (mKeyboard->WasKeyPressedThisFrame(Keys::NumPad8))
							PlayerManager::GetInstance()->getPlayer(i)->setHighlightedPlanet(Planet::Neighbor::UP);
						else if (mKeyboard->WasKeyPressedThisFrame(Keys::NumPad5))
							PlayerManager::GetInstance()->getPlayer(i)->setHighlightedPlanet(Planet::Neighbor::DOWN);

						if (mKeyboard->WasKeyPressedThisFrame(Keys::NumPad7))
							PlayerManager::GetInstance()->getPlayer(i)->changePercentageSelection();

						if (mKeyboard->WasKeyPressedThisFrame(Keys::NumPad9))
							PlayerManager::GetInstance()->getPlayer(i)->selectPlanet();
					}

					PlayerManager::GetInstance()->getPlayer(i)->Update(mTimer);
				}
			}

			// Game Over / Restart
			{
				StateManager* stateManager = StateManager::GetInstance();
				const StateManager::State state = stateManager->getState();

				if (state == StateManager::State::RUNNING)
				{
					if (checkGameOver()) stateManager->setState(StateManager::State::GAME_OVER);
				}
				else if (state == StateManager::State::RESTART)
				{
					PlayerManager::GetInstance()->shutdown();
					PlanetManager::GetInstance()->deletePlanets();

					init();
				}
			}
		});
	}

	// Renders the current frame according to the current application state.
	// Returns true if the frame was rendered and is ready to be displayed.
	bool GameMain::Render()
	{
		// Don't try to render anything before the first Update.
		if (mTimer.GetFrameCount() == 0)
		{
			return false;
		}

		auto context = mDeviceResources->GetD3DDeviceContext();

		// Reset the viewport to target the whole screen.
		auto viewport = mDeviceResources->GetScreenViewport();
		context->RSSetViewports(1, &viewport);

		// Reset render targets to the screen.
		ID3D11RenderTargetView *const targets[1] = { mDeviceResources->GetBackBufferRenderTargetView() };
		context->OMSetRenderTargets(1, targets, mDeviceResources->GetDepthStencilView());

		// Clear the back buffer and depth stencil view.
		context->ClearRenderTargetView(mDeviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::Black);
		context->ClearDepthStencilView(mDeviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		for (auto& component : mComponents)
		{
			auto drawableComponent = dynamic_pointer_cast<DrawableGameComponent>(component);
			if (drawableComponent != nullptr && drawableComponent->Visible())
			{
				drawableComponent->Render(mTimer);
			}
		}

		return true;
	}

	// Notifies renderers that device resources need to be released.
	void GameMain::OnDeviceLost()
	{
		for (auto& component : mComponents)
		{
			component->ReleaseDeviceDependentResources();
		}
	}

	// Notifies renderers that device resources may now be recreated.
	void GameMain::OnDeviceRestored()
	{
		IntializeResources();
	}

	void GameMain::IntializeResources()
	{
		for (auto& component : mComponents)
		{
			component->CreateDeviceDependentResources();
		}

		CreateWindowSizeDependentResources();
	}

	bool GameMain::checkGameOver()
	{
		bool8_t gameOver = false;

		PlayerManager* playerManager = PlayerManager::GetInstance();
		PlanetManager* planetManager = PlanetManager::GetInstance();

		StateManager::Winner winner = StateManager::Winner::INVAL_WINNER;

		for (int32_t i = 0; i < PlayerManager::NUM_PLAYERS; i++)
		{
			gameOver = (planetManager->getNumPlanets(i) == 0) && (playerManager->getPlayerTravellingShips(i) == 0);

			if (gameOver)
			{
				if (i == 0) winner = StateManager::Winner::PLAYER_TWO;
				else if (i == 1) winner = StateManager::Winner::PLAYER_ONE;
				break;
			}
		}

		StateManager::GetInstance()->setWinner(winner);

		return gameOver;
	}
	void GameMain::init()
	{
		StateManager::GetInstance()->init();
		PlanetManager::GetInstance()->InitializePlanets();
		PlayerManager::GetInstance()->init();
	}
	void GameMain::shutdown()
	{
		StateManager::GetInstance()->shutdown();
		PlayerManager::GetInstance()->shutdown();
		PlanetManager::GetInstance()->deletePlanets();
	}
}