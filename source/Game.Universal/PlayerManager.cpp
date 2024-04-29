#include "pch.h"
#include "PlayerManager.h"
#include "StateManager.h"
#include "PlanetManager.h"

using namespace std;
using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	/* Constants */

	const uchar8_t PlayerManager::NUM_PLAYERS = 2;
	const DirectX::XMFLOAT4 PlayerManager::PLAYER_ONE_COLOR = XMFLOAT4(0, 0, 1, 1.0f);
	const DirectX::XMFLOAT4 PlayerManager::PLAYER_TWO_COLOR = XMFLOAT4(1, 0, 0, 1.0f);
	const float_t PlayerManager::PLAYER_START_SHIPCOUNT = 50.0f;
	const char8_t PlayerManager::PLAYER_ONE_ID = 0;
	const char8_t PlayerManager::PLAYER_TWO_ID = 1;
	const float_t PlayerManager::HIGHLIGHT_OFFSET_1 = 7;
	const float_t PlayerManager::HIGHLIGHT_OFFSET_2 = 10;
	const int PlayerManager::HIGHLIGHT_THICKNESS = 8;

	const uint32_t PlayerManager::CircleResolution = 32;
	const uint32_t PlayerManager::LineCircleVertexCount = PlayerManager::CircleResolution + 1;
	const uint32_t PlayerManager::SolidCircleVertexCount = (PlayerManager::CircleResolution + 1) * 2;

	PlayerManager* PlayerManager::sInstance = nullptr;
	/* Methods */

	PlayerManager* PlayerManager::CreateInstance()
	{
		assert(sInstance != nullptr);

		return sInstance;
	}

	PlayerManager::PlayerManager(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<DX::Camera>& camera) :
		DrawableGameComponent(deviceResources, camera),
		mLoadingComplete(false)
	{
		sInstance = this;
		CreateDeviceDependentResources();
	}

	std::shared_ptr<Field> PlayerManager::ActiveField() const
	{
		return mActiveField;
	}

	void PlayerManager::SetActiveField(const shared_ptr<Field>& field)
	{
		mActiveField = field;
	}

	void PlayerManager::init()
	{
		PlanetManager* planetManager = PlanetManager::GetInstance();
		//uint32_t numPlanets = planetManager->getNumPlanets();

		std::shared_ptr<Planet> planetPtr = nullptr;


		// Player One

		mPlayers.push_back(new Player(0));
		mPlayers[0]->setColor(PLAYER_ONE_COLOR);

		planetPtr = planetManager->getPlanetAt(0);
		planetPtr->SetColor(PLAYER_ONE_COLOR);
		planetPtr->SetShipCount(PLAYER_START_SHIPCOUNT);
		mPlayers[0]->setHighlightedPlanet(planetPtr);


		// Player Two

		mPlayers.push_back(new Player(1));
		mPlayers[1]->setColor(PLAYER_TWO_COLOR);

		planetPtr = planetManager->getPlanetAt(1);
		planetPtr->SetColor(PLAYER_TWO_COLOR);
		planetPtr->SetShipCount(PLAYER_START_SHIPCOUNT);
		mPlayers[1]->setHighlightedPlanet(planetPtr);
	}

	void PlayerManager::shutdown()
	{
		for (size_t i = 0; i < mPlayers.size(); i++)
		{
			mPlayers.at(i)->shutdown();
			delete(mPlayers.at(i));
		}
		mPlayers.clear();
	}

	void PlayerManager::Update(const StepTimer& timer)
	{
		if (StateManager::GetInstance()->getState() != StateManager::State::RUNNING)
			return;

		for (const auto& player : mPlayers)
		{
			player->Update(timer);
		}
	}

	void PlayerManager::Render(const DX::StepTimer& timer)
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
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

		direct3DDeviceContext->VSSetConstantBuffers(0, 1, mVSCBufferPerObject.GetAddressOf());
		direct3DDeviceContext->PSSetConstantBuffers(0, 1, mPSCBufferPerObject.GetAddressOf());

		for (const auto& player : mPlayers)
		{
			DrawCircle(*player);

			if (!player->GetFleetList().empty())
			{
				for (const auto& fleet : player->GetFleetList())
				{
					DrawShips(*fleet);
				}
			}
		}
	}

	void PlayerManager::CreateDeviceDependentResources()
	{
		auto loadVSTask = ReadDataAsync(L"ShapeRendererVS.cso");
		auto loadPSTask = ReadDataAsync(L"ShapeRendererPS.cso");

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
					VertexPosition::InputElements,
					VertexPosition::InputElementCount,
					&fileData[0],
					fileData.size(),
					mInputLayout.ReleaseAndGetAddressOf()
				)
			);

			CD3D11_BUFFER_DESC constantBufferDesc(sizeof(XMFLOAT4X4), D3D11_BIND_CONSTANT_BUFFER);
			ThrowIfFailed(
				mDeviceResources->GetD3DDevice()->CreateBuffer(
					&constantBufferDesc,
					nullptr,
					mVSCBufferPerObject.ReleaseAndGetAddressOf()
				)
			);
		});

		// After the pixel shader file is loaded, create the shader and constant buffer.
		auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
			ThrowIfFailed(
				mDeviceResources->GetD3DDevice()->CreatePixelShader(
					&fileData[0],
					fileData.size(),
					nullptr,
					mPixelShader.ReleaseAndGetAddressOf()
				)
			);

			CD3D11_BUFFER_DESC constantBufferDesc(sizeof(XMFLOAT4), D3D11_BIND_CONSTANT_BUFFER);
			ThrowIfFailed(
				mDeviceResources->GetD3DDevice()->CreateBuffer(
					&constantBufferDesc,
					nullptr,
					mPSCBufferPerObject.ReleaseAndGetAddressOf()
				)
			);
		});

		auto createVerticesAndBallsTask = (createPSTask && createVSTask).then([this]() {
			InitializeLineVertices();
			InitializeCircleVertices();
		});
			   
		// Once the cube is loaded, the object is ready to be rendered.
		createVerticesAndBallsTask.then([this]() {
			mLoadingComplete = true;
		});
	}

	void PlayerManager::ReleaseDeviceDependentResources()
	{
		mLoadingComplete = false;
		mVertexShader.Reset();
		mPixelShader.Reset();
		mInputLayout.Reset();
		mLineVertexBuffer.Reset();
		mCircleVertexBuffer.Reset();
		mVSCBufferPerObject.Reset();
		mPSCBufferPerObject.Reset();
	}

	Player* PlayerManager::getPlayer(int32_t playerID)
	{
		return mPlayers.at(playerID);
	}

	std::vector<Player*> PlayerManager::GetPlayers()
	{
		return mPlayers;
	}


	int PlayerManager::getPlayerTravellingShips(int32_t  playerID)
	{
		return getPlayer(playerID)->getNumberOfTravellingShips();
	}

	int32_t PlayerManager::getPlayerPercentageSelect(int32_t playerID)
	{
		return getPlayer(playerID)->GetPecentageSelection();
	}
	void PlayerManager::InitializeLineVertices()
	{
		// Create a vertex buffer for rendering a box
		D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
		const uint32_t boxVertexCount = 4;
		vertexBufferDesc.ByteWidth = sizeof(VertexPosition) * boxVertexCount;
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, nullptr, mLineVertexBuffer.ReleaseAndGetAddressOf()));

		// Create an index buffer for the box (line strip)
		uint32_t indices[] =
		{
			0, 1, 2, 3, 0
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

	void PlayerManager::InitializeCircleVertices()
	{
		const float increment = XM_2PI / CircleResolution;

		VertexPosition vertices[LineCircleVertexCount];

		for (int i = 0; i < CircleResolution; i++)
		{
			VertexPosition& vertex = vertices[i];

			vertex.Position.x = cosf(i * increment);
			vertex.Position.y = sinf(i * increment);
			vertex.Position.z = 0.0f;
			vertex.Position.w = 1.0f;
		}

		// Closing line to complete the circle
		vertices[CircleResolution] = VertexPosition(vertices[0]);

		// Axis line for visualizing rotation
		//vertices[CircleResolution + 1] = VertexPosition(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

		D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
		vertexBufferDesc.ByteWidth = sizeof(VertexPosition) * LineCircleVertexCount;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData = { 0 };
		vertexSubResourceData.pSysMem = vertices;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mCircleVertexBuffer.ReleaseAndGetAddressOf()));
	}
	void PlayerManager::DrawCircle(const Player& player)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

		static const UINT stride = sizeof(VertexPosition);
		static const UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mCircleVertexBuffer.GetAddressOf(), &stride, &offset);

		auto highlightPlanet = player.GetHighlightedPlanet();
		float radius = highlightPlanet->Radius() + (player.getPlayerID() == PLAYER_ONE_ID ? HIGHLIGHT_OFFSET_1 : HIGHLIGHT_OFFSET_2);
		int thickness = HIGHLIGHT_THICKNESS;
		for (int i = 0; i < thickness; ++i)
		{
			XMMATRIX scale = XMMatrixScaling(radius - i*.5f, radius - i * .5f, radius - i * .5f);
			XMMATRIX translation = XMMatrixTranslation(highlightPlanet->GetPlanetPosition().x - OrthographicCamera::DefaultViewWidth / 2, highlightPlanet->GetPlanetPosition().y - OrthographicCamera::DefaultViewHeight / 2, 0.0f);
			XMMATRIX wvp = XMMatrixMultiply(scale, translation);
			wvp *= mCamera->ViewProjectionMatrix();
			wvp = XMMatrixTranspose(wvp);

			direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, reinterpret_cast<const float*>(wvp.r), 0, 0);

			direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &player.Color(), 0, 0);

			direct3DDeviceContext->Draw(LineCircleVertexCount, 0);			
		}


		if (player.GetSelectedPlanet() && player.GetSelectedPlanet() != player.GetHighlightedPlanet())
		{
			const float_t selectRadius = player.GetSelectedPlanet()->Radius() + (player.getPlayerID() == PLAYER_ONE_ID ? HIGHLIGHT_OFFSET_1 : HIGHLIGHT_OFFSET_2);
			const Coord2D selectPos = player.GetSelectedPlanet()->GetPlanetPosition();

			for (int i = 0; i < thickness; ++i)
			{
				XMMATRIX scale = XMMatrixScaling(selectRadius - i * .5f, selectRadius - i * .5f, selectRadius - i * .5f);
				XMMATRIX translation = XMMatrixTranslation(selectPos.x - OrthographicCamera::DefaultViewWidth / 2, selectPos.y - OrthographicCamera::DefaultViewHeight / 2, 0.0f);
				XMMATRIX wvp = XMMatrixMultiply(scale, translation);
				wvp *= mCamera->ViewProjectionMatrix();
				wvp = XMMatrixTranspose(wvp);

				direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, reinterpret_cast<const float*>(wvp.r), 0, 0);

				direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &player.Color(), 0, 0);

				direct3DDeviceContext->Draw(LineCircleVertexCount, 0);
			}

			DrawLine(player);
		}

	}
	void PlayerManager::DrawLine(const Player& player)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		static const UINT stride = sizeof(VertexPosition);
		static const UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mLineVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

		const XMMATRIX wvp = XMMatrixTranspose(mCamera->ViewProjectionMatrix());
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, reinterpret_cast<const float*>(wvp.r), 0, 0);
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, mVSCBufferPerObject.GetAddressOf());
		direct3DDeviceContext->PSSetConstantBuffers(0, 1, mPSCBufferPerObject.GetAddressOf());

		const Coord2D highlightPos = player.GetHighlightedPlanet()->GetPlanetPosition();
		const Coord2D selectPos = player.GetSelectedPlanet()->GetPlanetPosition();

		VertexPosition vertices[] =
		{
			// Point 1
			VertexPosition(XMFLOAT4
			(
				highlightPos.x - OrthographicCamera::DefaultViewWidth / 2,
				highlightPos.y + (player.getPlayerID() == PLAYER_ONE_ID ? -1 : 1) - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),

			// Point 2
			VertexPosition(XMFLOAT4
			(
				selectPos.x - OrthographicCamera::DefaultViewWidth / 2,
				selectPos.y + (player.getPlayerID() == PLAYER_ONE_ID ? -1 : 1) - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),

			// Point 1
			VertexPosition(XMFLOAT4
			(
				highlightPos.x - OrthographicCamera::DefaultViewWidth / 2,
				highlightPos.y + (player.getPlayerID() == PLAYER_ONE_ID ? -1 : 1) - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),

			// Point 2
			VertexPosition(XMFLOAT4
			(
				selectPos.x - OrthographicCamera::DefaultViewWidth / 2,
				selectPos.y + (player.getPlayerID() == PLAYER_ONE_ID ? -1 : 1) - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),
		};

		direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		uint32_t vertexCount = ARRAYSIZE(vertices);
		D3D11_MAPPED_SUBRESOURCE mappedSubResource;
		direct3DDeviceContext->Map(mLineVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubResource);
		memcpy(mappedSubResource.pData, vertices, sizeof(VertexPosition) * vertexCount);
		direct3DDeviceContext->Unmap(mLineVertexBuffer.Get(), 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &player.Color(), 0, 0);

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}

	void PlayerManager::DrawShips(const Fleet& fleet)
	{
		for (const auto& ship : fleet.GetShipList())
		{
			DrawShip(*ship);
			if (StateManager::GetInstance()->getMode() == StateManager::Mode::DEBUG)
			{
				DrawDestinationLines(*ship);
				DrawDirectionLines(*ship);
			}
		}
	}

	void PlayerManager::DrawShip(const Ship& ship)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		static const UINT stride = sizeof(VertexPosition);
		static const UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mLineVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

		const XMMATRIX wvp = XMMatrixTranspose(mCamera->ViewProjectionMatrix());
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, reinterpret_cast<const float*>(wvp.r), 0, 0);
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, mVSCBufferPerObject.GetAddressOf());
		direct3DDeviceContext->PSSetConstantBuffers(0, 1, mPSCBufferPerObject.GetAddressOf());
		
		VertexPosition vertices[] =
		{
			// Point 1
			VertexPosition(XMFLOAT4
			(
				ship.GetFrontVertex().x - OrthographicCamera::DefaultViewWidth / 2,
				ship.GetFrontVertex().y - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),

			// Point 2
			VertexPosition(XMFLOAT4
			(
				ship.GetBackLeftVertex().x - OrthographicCamera::DefaultViewWidth / 2,
				ship.GetBackLeftVertex().y - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),

			// Point 3
			VertexPosition(XMFLOAT4
			(
				ship.GetBackRightVertex().x - OrthographicCamera::DefaultViewWidth / 2,
				ship.GetBackRightVertex().y - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),

			// Point 1
			VertexPosition(XMFLOAT4
			(
				ship.GetFrontVertex().x - OrthographicCamera::DefaultViewWidth / 2,
				ship.GetFrontVertex().y - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),
		};

		direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		uint32_t vertexCount = ARRAYSIZE(vertices);
		D3D11_MAPPED_SUBRESOURCE mappedSubResource;
		direct3DDeviceContext->Map(mLineVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubResource);
		memcpy(mappedSubResource.pData, vertices, sizeof(VertexPosition) * vertexCount);
		direct3DDeviceContext->Unmap(mLineVertexBuffer.Get(), 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &ship.Color(), 0, 0);

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}

	void PlayerManager::DrawDestinationLines(const Ship& ship)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		static const UINT stride = sizeof(VertexPosition);
		static const UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mLineVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

		const XMMATRIX wvp = XMMatrixTranspose(mCamera->ViewProjectionMatrix());
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, reinterpret_cast<const float*>(wvp.r), 0, 0);
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, mVSCBufferPerObject.GetAddressOf());
		direct3DDeviceContext->PSSetConstantBuffers(0, 1, mPSCBufferPerObject.GetAddressOf());

		VertexPosition vertices[] =
		{
			// Point 1
			VertexPosition(XMFLOAT4
			(
				ship.GetPosition().x - OrthographicCamera::DefaultViewWidth / 2,
				ship.GetPosition().y - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),

			// Point 2
			VertexPosition(XMFLOAT4
			(
				ship.GetDestination().x - OrthographicCamera::DefaultViewWidth / 2,
				ship.GetDestination().y - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),

			// Point 1
			VertexPosition(XMFLOAT4
			(
				ship.GetPosition().x - OrthographicCamera::DefaultViewWidth / 2,
				ship.GetPosition().y - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),

			// Point 2
			VertexPosition(XMFLOAT4
			(
				ship.GetDestination().x - OrthographicCamera::DefaultViewWidth / 2,
				ship.GetDestination().y - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),
		};

		direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		uint32_t vertexCount = ARRAYSIZE(vertices);
		D3D11_MAPPED_SUBRESOURCE mappedSubResource;
		direct3DDeviceContext->Map(mLineVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubResource);
		memcpy(mappedSubResource.pData, vertices, sizeof(VertexPosition) * vertexCount);
		direct3DDeviceContext->Unmap(mLineVertexBuffer.Get(), 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &Colors::Magenta, 0, 0);

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}
	void PlayerManager::DrawDirectionLines(const Ship& ship)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		static const UINT stride = sizeof(VertexPosition);
		static const UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mLineVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

		const XMMATRIX wvp = XMMatrixTranspose(mCamera->ViewProjectionMatrix());
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, reinterpret_cast<const float*>(wvp.r), 0, 0);
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, mVSCBufferPerObject.GetAddressOf());
		direct3DDeviceContext->PSSetConstantBuffers(0, 1, mPSCBufferPerObject.GetAddressOf());

		VertexPosition vertices[] =
		{
			// Point 1
			VertexPosition(XMFLOAT4
			(
				ship.GetPosition().x - OrthographicCamera::DefaultViewWidth / 2,
				ship.GetPosition().y - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),

			// Point 2
			VertexPosition(XMFLOAT4
			(
				ship.GetPosition().x + ship.GetVelocity().x *5 - OrthographicCamera::DefaultViewWidth / 2,
				ship.GetPosition().y + ship.GetVelocity().y * 5 - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),

			// Point 1
			VertexPosition(XMFLOAT4
			(
				ship.GetPosition().x - OrthographicCamera::DefaultViewWidth / 2,
				ship.GetPosition().y - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),

			// Point 2
			VertexPosition(XMFLOAT4
			(
				ship.GetPosition().x + ship.GetVelocity().x * 5 - OrthographicCamera::DefaultViewWidth / 2,
				ship.GetPosition().y + ship.GetVelocity().y * 5 - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),
		};

		direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		uint32_t vertexCount = ARRAYSIZE(vertices);
		D3D11_MAPPED_SUBRESOURCE mappedSubResource;
		direct3DDeviceContext->Map(mLineVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubResource);
		memcpy(mappedSubResource.pData, vertices, sizeof(VertexPosition) * vertexCount);
		direct3DDeviceContext->Unmap(mLineVertexBuffer.Get(), 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &Colors::Green, 0, 0);

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}
}