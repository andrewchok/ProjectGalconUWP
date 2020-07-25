#include "pch.h"
#include "PlanetManager.h"
#include "Planet.h"
#include "PlayerManager.h"
#include "StateManager.h"

using namespace std;
using namespace DirectX;
using namespace DX;
using namespace Microsoft::WRL;

namespace DirectXGame
{
	/* Constants */

	const int32_t PlanetManager::NUM_PLANETS = 30;
	const float_t PlanetManager::INTER_PLANET_OFFSET = 10.0f;
	const float_t PlanetManager::HOME_PLANET_RADIUS = Planet::MAX_RADIUS / 2;

	PlanetManager* PlanetManager::sInstance = nullptr;

	const uint32_t PlanetManager::CircleResolution = 32;
	const uint32_t PlanetManager::LineCircleVertexCount = PlanetManager::CircleResolution + 2;
	const uint32_t PlanetManager::SolidCircleVertexCount = (PlanetManager::CircleResolution + 1) * 2;

	PlanetManager* PlanetManager::GetInstance()
	{
		assert(sInstance != nullptr);

		return sInstance;
	}

	PlanetManager::PlanetManager(const shared_ptr<DX::DeviceResources>& deviceResources, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(deviceResources, camera),
		m_text(L""),
		mLoadingComplete(false)
	{
		if (sInstance == nullptr)
		{
			sInstance = this;
		}

#pragma region Text things


		ZeroMemory(&m_textMetrics, sizeof(DWRITE_TEXT_METRICS));

		// Create device independent resources
		ComPtr<IDWriteTextFormat> textFormat;
		DX::ThrowIfFailed(
			mDeviceResources->GetDWriteFactory()->CreateTextFormat(
				L"Segoe UI",
				nullptr,
				DWRITE_FONT_WEIGHT_LIGHT,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				20.0f,
				L"en-US",
				&textFormat
			)
		);

		DX::ThrowIfFailed(
			textFormat.As(&m_textFormat)
		);

		DX::ThrowIfFailed(
			m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
		);

		DX::ThrowIfFailed(
			mDeviceResources->GetD2DFactory()->CreateDrawingStateBlock(&m_stateBlock)
		);

#pragma endregion

		CreateDeviceDependentResources();
	}

	std::shared_ptr<Field> PlanetManager::ActiveField() const
	{
		return mActiveField;
	}

	void PlanetManager::SetActiveField(const shared_ptr<Field>& field)
	{
		mActiveField = field;
	}

	void PlanetManager::CreateDeviceDependentResources()
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
			InitializeTriangleVertices();
			//InitializePlanets();
		});

#pragma region Text Things

		DX::ThrowIfFailed(
			mDeviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_whiteBrush)
		);
#pragma endregion


		// Once the cube is loaded, the object is ready to be rendered.
		createVerticesAndBallsTask.then([this]() {
			mLoadingComplete = true;
		});
	}

	void PlanetManager::ReleaseDeviceDependentResources()
	{
		mLoadingComplete = false;
		mVertexShader.Reset();
		mPixelShader.Reset();
		mInputLayout.Reset();
		mLineVertexBuffer.Reset();
		mTriangleVertexBuffer.Reset();
		mVSCBufferPerObject.Reset();
		mPSCBufferPerObject.Reset();

		m_whiteBrush.Reset();
	}

	void PlanetManager::Update(const StepTimer& timer)
	{
		if (StateManager::GetInstance()->getState() != StateManager::State::RUNNING)
			return;

		for (const auto& planet : mPlanets)
		{
			planet->Update(timer);
		}
	}

	void PlanetManager::Render(const StepTimer& timer)
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

		for (const auto& planet : mPlanets)
		{
			DrawPlanet(*planet); 
			 
			if (StateManager::GetInstance()->getState() == StateManager::State::RUNNING)
				DrawText(*planet);
		}
	}
	
	void PlanetManager::deletePlanets()
	{
		mPlanets.clear();
	}

	uint32_t PlanetManager::getNumPlanets(int32_t playerID)
	{
		uint32_t count = 0;

		DirectX::XMFLOAT4 color = Vector4Helper::One;

		if (playerID == 0)
			color = PlayerManager::PLAYER_ONE_COLOR;
		else if (playerID == 1)
			color = PlayerManager::PLAYER_TWO_COLOR;

		for (size_t i = 0; i < mPlanets.size(); i++)
		{
			if (mPlanets.at(i)->Color().x == color.x && mPlanets.at(i)->Color().y == color.y && mPlanets.at(i)->Color().z == color.z)
			{
				count++;
			}
		}

		return count;
	}

	std::vector<std::shared_ptr<Planet>>* PlanetManager::getPlanets()
	{
		return &mPlanets;
	}

	std::vector<std::shared_ptr<Planet>>* PlanetManager::getPlanets(int32_t  playerID)
	{
		std::vector<std::shared_ptr<Planet>>* planets = new std::vector<std::shared_ptr<Planet>>();

		DirectX::XMFLOAT4 color = Vector4Helper::One;

		if (playerID == 0)
			color = PlayerManager::PLAYER_ONE_COLOR;
		else if (playerID == 1)
			color = PlayerManager::PLAYER_TWO_COLOR;

		for (size_t i = 0; i < mPlanets.size(); i++)
		{
			if (mPlanets.at(i)->Color().x == color.x && mPlanets.at(i)->Color().y == color.y && mPlanets.at(i)->Color().z == color.z)
			{
				planets->push_back(mPlanets.at(i));
			}
		}

		return planets;
	}

	std::shared_ptr<Planet> PlanetManager::getPlanetAt(int32_t index)
	{
		return mPlanets.at(index);
	}

	void PlanetManager::DrawPlanet(const Planet& planet)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		static const UINT stride = sizeof(VertexPosition);
		static const UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mTriangleVertexBuffer.GetAddressOf(), &stride, &offset);

		XMMATRIX scale = XMMatrixScaling(planet.Radius(), planet.Radius(), planet.Radius());
		XMMATRIX translation = XMMatrixTranslation(planet.GetPlanetPosition().x - OrthographicCamera::DefaultViewWidth / 2, planet.GetPlanetPosition().y - OrthographicCamera::DefaultViewHeight / 2, 0.0f);
		XMMATRIX wvp = XMMatrixMultiply(scale, translation);
		wvp *= mCamera->ViewProjectionMatrix();
		wvp = XMMatrixTranspose(wvp);

		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, reinterpret_cast<const float*>(wvp.r), 0, 0);

		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &planet.Color(), 0, 0);

		direct3DDeviceContext->Draw(SolidCircleVertexCount, 0);


		// Check state before drawing debug neighbor lines
		if (StateManager::GetInstance()->getMode() == StateManager::Mode::DEBUG )
			DrawNeighborLines(planet);
	}

	void PlanetManager::DrawText(const Planet& planet)
	{
		// Update display text.
		float_t fps = planet.ShipCount();

		m_text = std::to_wstring(static_cast<int32_t>(fps));

		ComPtr<IDWriteTextLayout> textLayout;
		DX::ThrowIfFailed(
			mDeviceResources->GetDWriteFactory()->CreateTextLayout(
				m_text.c_str(),
				(uint32)m_text.length(),
				m_textFormat.Get(),
				fps >= 1000 ? 43 : fps >= 100 ? 32.0f : fps >= 10 ? 22 : 11, // Max width of the input text.
				10.0f, // Max height of the input text.
				&textLayout
			)
		);

		DX::ThrowIfFailed(
			textLayout.As(&m_textLayout)
		);

		DX::ThrowIfFailed(
			m_textLayout->GetMetrics(&m_textMetrics)
		);

		ID2D1DeviceContext* context = mDeviceResources->GetD2DDeviceContext();
		Windows::Foundation::Size logicalSize = mDeviceResources->GetLogicalSize();

		context->SaveDrawingState(m_stateBlock.Get());
		context->BeginDraw();

		// Position on the bottom right corner
		D2D1::Matrix3x2F screenTranslation = D2D1::Matrix3x2F::Translation(
			logicalSize.Width * (planet.GetPlanetPosition().x / OrthographicCamera::DefaultViewWidth) - (m_textMetrics.layoutWidth / 2),
			logicalSize.Height * (1 - planet.GetPlanetPosition().y / OrthographicCamera::DefaultViewHeight) - (m_textMetrics.height / 2)
		);

		XMMATRIX scale = XMMatrixScaling(1, 1, 1);
		XMMATRIX translation = XMMatrixTranslation(planet.GetPlanetPosition().x - OrthographicCamera::DefaultViewWidth / 2, planet.GetPlanetPosition().y - OrthographicCamera::DefaultViewHeight / 2, 0.0f);
		XMMATRIX wvp = XMMatrixMultiply(scale, translation);
		wvp *= mCamera->ViewProjectionMatrix();
		wvp = XMMatrixTranspose(wvp);

		context->SetTransform(screenTranslation);// * mDeviceResources->GetOrientationTransform2D());

		DX::ThrowIfFailed(
			m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING)
		);

		context->DrawTextLayout(
			D2D1::Point2F(0.f, 0.f),
			m_textLayout.Get(),
			m_whiteBrush.Get()
		);

		// Ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
		// is lost. It will be handled during the next call to Present.
		HRESULT hr = context->EndDraw();
		if (hr != D2DERR_RECREATE_TARGET)
		{
			DX::ThrowIfFailed(hr);
		}

		context->RestoreDrawingState(m_stateBlock.Get());
	}

	void PlanetManager::DrawNeighborLines(const Planet& planet)
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

		if (planet.GetNeighbor(Planet::Neighbor::LEFT))
		{
			DrawLineToNeighbor(planet, planet.GetNeighbor(Planet::Neighbor::LEFT));
		}

		if (planet.GetNeighbor(Planet::Neighbor::RIGHT))
		{
			DrawLineToNeighbor(planet, planet.GetNeighbor(Planet::Neighbor::RIGHT));
		}

		if (planet.GetNeighbor(Planet::Neighbor::UP))
		{
			DrawLineToNeighbor(planet, planet.GetNeighbor(Planet::Neighbor::UP));
		}

		if (planet.GetNeighbor(Planet::Neighbor::DOWN))
		{
			DrawLineToNeighbor(planet, planet.GetNeighbor(Planet::Neighbor::DOWN));
		}
	}

	void PlanetManager::DrawLineToNeighbor(const Planet& planet, const std::shared_ptr<Planet>& neighbor)
	{
		VertexPosition vertices[] =
		{
			// Point 1
			VertexPosition(XMFLOAT4
			(
				neighbor->GetPlanetPosition().x - OrthographicCamera::DefaultViewWidth / 2,
				neighbor->GetPlanetPosition().y - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),

			// Point 2
			VertexPosition(XMFLOAT4
			(
				planet.GetPlanetPosition().x - OrthographicCamera::DefaultViewWidth / 2,
				planet.GetPlanetPosition().y - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),

			// Point 1
			VertexPosition(XMFLOAT4
			(
				neighbor->GetPlanetPosition().x - OrthographicCamera::DefaultViewWidth / 2,
				neighbor->GetPlanetPosition().y - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),

			// Point 2
			VertexPosition(XMFLOAT4
			(
				planet.GetPlanetPosition().x - OrthographicCamera::DefaultViewWidth / 2,
				planet.GetPlanetPosition().y - OrthographicCamera::DefaultViewHeight / 2,
				0.0f,
				1.0f)
			),
		};

		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		uint32_t vertexCount = ARRAYSIZE(vertices);
		D3D11_MAPPED_SUBRESOURCE mappedSubResource;
		direct3DDeviceContext->Map(mLineVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubResource);
		memcpy(mappedSubResource.pData, vertices, sizeof(VertexPosition) * vertexCount);
		direct3DDeviceContext->Unmap(mLineVertexBuffer.Get(), 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &Colors::AntiqueWhite[0], 0, 0);

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}

	void PlanetManager::initPlanetNeighbors()
	{
		for (uint32_t i = 0; i < NUM_PLANETS; i++)
		{
			std::shared_ptr<Planet> leftNeighbor = nullptr;
			std::shared_ptr<Planet> rightNeighbor = nullptr;
			std::shared_ptr<Planet> upNeighbor = nullptr;
			std::shared_ptr<Planet> downNeighbor = nullptr;

			for (uint32_t j = 0; j < NUM_PLANETS; j++)
			{
				// Guards against setting planet as it's own neighbor
				if (i == j) continue;

				const Coord2D curPos = mPlanets.at(i)->GetPlanetPosition();
				const Coord2D testPos = mPlanets.at(j)->GetPlanetPosition();
				const Coord2D testDist = curPos - testPos;
				const Coord2D testDirUnit = testDist / testDist.magnitude();

				Coord2D dirUnit = { 0, 0 };
				float_t angle = 0;

				// LEFT NEIGHBOR CHECK

				dirUnit = { 1, 0 };
				angle = acosf(testDirUnit.dot(dirUnit));

				if (leftNeighbor != nullptr)
				{
					const Coord2D curLeftPos = leftNeighbor->GetPlanetPosition();
					const Coord2D curDist = curPos - curLeftPos;

					if (angle < (PI / 4.0f) && curDist.magnitude() > testDist.magnitude())
					{
						leftNeighbor = mPlanets.at(j);
					}
				}
				else if (angle < (PI / 4.0f))
				{
					leftNeighbor = mPlanets.at(j);
				}


				// RIGHT NEIGHBOR CHECK

				dirUnit = { -1, 0 };
				angle = acosf(testDirUnit.dot(dirUnit));

				if (rightNeighbor != nullptr)
				{
					const Coord2D curRightPos = rightNeighbor->GetPlanetPosition();
					const Coord2D curDist = curPos - curRightPos;

					if (angle < (PI / 4.0f) && curDist.magnitude() > testDist.magnitude())
					{
						rightNeighbor = mPlanets.at(j);
					}
				}
				else if (angle < (PI / 4.0f))
				{
					rightNeighbor = mPlanets.at(j);
				}


				// UP NEIGHBOR CHECK

				dirUnit = { 0, -1 };
				angle = acosf(testDirUnit.dot(dirUnit));

				if (upNeighbor != nullptr)
				{
					const Coord2D curUpPos = upNeighbor->GetPlanetPosition();
					const Coord2D curDist = curPos - curUpPos;

					if (angle < (PI / 4.0f) && curDist.magnitude() > testDist.magnitude())
					{
						upNeighbor = mPlanets.at(j);
					}
				}
				else if (angle <= (PI / 4.0f))
				{
					upNeighbor = mPlanets.at(j);
				}


				// DOWN NEIGHBOR CHECK

				dirUnit = { 0, 1 };
				angle = acosf(testDirUnit.dot(dirUnit));

				if (downNeighbor != nullptr)
				{
					const Coord2D curDownPos = downNeighbor->GetPlanetPosition();
					const Coord2D curDist = curPos - curDownPos;

					if (angle < (PI / 4.0f) && curDist.magnitude() > testDist.magnitude())
					{
						downNeighbor = mPlanets.at(j);
					}
				}
				else if (angle <= (PI / 4.0f))
				{
					downNeighbor = mPlanets.at(j);
				}
			}

			mPlanets.at(i)->SetNeighbors(leftNeighbor, rightNeighbor, upNeighbor, downNeighbor);
		}
	}
	
	void PlanetManager::InitializeLineVertices()
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

	void PlanetManager::InitializeTriangleVertices()
	{
		const float increment = XM_2PI / CircleResolution;
		const XMFLOAT4 center(0.0f, 0.0f, 0.0f, 1.0f);

		vector<VertexPosition> vertices;
		vertices.reserve(SolidCircleVertexCount);
		for (int i = 0; i <= CircleResolution; i++)
		{
			VertexPosition vertex;
			vertex.Position.x = cosf(i * increment);
			vertex.Position.y = sinf(i * increment);
			vertex.Position.z = 0.0f;
			vertex.Position.w = 1.0f;

			vertices.push_back(vertex);
			vertices.push_back(center);
		}

		assert(vertices.size() == SolidCircleVertexCount);

		D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
		vertexBufferDesc.ByteWidth = sizeof(VertexPosition) * static_cast<uint32_t>(vertices.size());
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData = { 0 };
		vertexSubResourceData.pSysMem = &vertices[0];
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mTriangleVertexBuffer.ReleaseAndGetAddressOf()));
	}

	void PlanetManager::InitializePlanets()
	{
		if (!mPlanets.empty()) return;
		printf("Hey initing the planet manager!\n");
		mPlanets.reserve(NUM_PLANETS);

		for (uint32_t i = 0; i < NUM_PLANETS; i++)
		{
			shared_ptr<Planet> newPlanet = nullptr;

			while (newPlanet == nullptr)
			{
				shared_ptr<Planet> testPlanet = make_shared<Planet>(*this);
				testPlanet->initRandom();

				const auto testCoord = testPlanet->Transform().Position();

				if (i < 2/*PlayerManagerC::NUM_PLAYERS*/) testPlanet->SetRadius(HOME_PLANET_RADIUS);

				const float_t testRadius = testPlanet->Radius();

				bool noCollisions = true;
				for (uint32_t prevIndex = 0; prevIndex < i; prevIndex++)
				{
					const auto planetPos = mPlanets.at(prevIndex)->Transform().Position();
					const float_t planetRadius = mPlanets.at(prevIndex)->Radius() + INTER_PLANET_OFFSET;

					if ((testCoord.x + testRadius >= planetPos.x - planetRadius)
						&& (testCoord.x - testRadius <= planetPos.x + planetRadius)
						&& (testCoord.y + testRadius >= planetPos.y - planetRadius)
						&& (testCoord.y - testRadius <= planetPos.y + planetRadius))
					{
						noCollisions = false;
						break;
					}
				}

				if (noCollisions)
				{
					newPlanet = testPlanet;
				}
			}

			mPlanets.push_back(newPlanet);
		}

		initPlanetNeighbors();
	}
}
