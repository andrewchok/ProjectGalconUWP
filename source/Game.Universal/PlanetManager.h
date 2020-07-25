#pragma once

#include "DrawableGameComponent.h"
#include <DirectXMath.h>
#include <vector>

namespace DirectXGame
{
	class Planet;
	//class Field;

	class PlanetManager final : public DX::DrawableGameComponent
	{
	public:
		/* Constants (defined in source file) */
		static const int32_t NUM_PLANETS;
		static const float_t INTER_PLANET_OFFSET;
		static const float_t HOME_PLANET_RADIUS;

	public:
		static PlanetManager* GetInstance();
		PlanetManager(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<DX::Camera>& camera);

		std::shared_ptr<Field> ActiveField() const;
		void SetActiveField(const std::shared_ptr<Field>& field);

		virtual void CreateDeviceDependentResources() override;
		virtual void ReleaseDeviceDependentResources() override;
		virtual void Update(const DX::StepTimer& timer) override;
		virtual void Render(const DX::StepTimer& timer) override;

		/*Planet things*/
		void deletePlanets();

		uint32_t getNumPlanets() noexcept { return NUM_PLANETS; }
		uint32_t getNumPlanets(int32_t  playerID);

		std::vector<std::shared_ptr<Planet>>* getPlanets();
		std::vector<std::shared_ptr<Planet>>* getPlanets(int32_t  playerID);
		std::shared_ptr<Planet> getPlanetAt(int32_t index);

		void InitializePlanets();
	private:
		void InitializeLineVertices();
		void InitializeTriangleVertices();
		void DrawPlanet(const Planet& planet);
		void DrawText(const Planet& planet);
		void DrawNeighborLines(const Planet& planet);
		void DrawLineToNeighbor(const Planet& planet, const std::shared_ptr<Planet>& neighbor);


		/*Planet things*/
		static PlanetManager* sInstance;
		void initPlanetNeighbors();
		/*Planet things End*/

		static const std::uint32_t CircleResolution;
		static const std::uint32_t LineCircleVertexCount;
		static const std::uint32_t SolidCircleVertexCount;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mLineVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mTriangleVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerObject;
		bool mLoadingComplete;
		std::vector<std::shared_ptr<Planet>> mPlanets;
		std::shared_ptr<Field> mActiveField;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
		std::uint32_t mIndexCount;

		// Text things
		std::wstring                                    m_text;
		DWRITE_TEXT_METRICS	                            m_textMetrics;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>    m_whiteBrush;
		Microsoft::WRL::ComPtr<ID2D1DrawingStateBlock1> m_stateBlock;
		Microsoft::WRL::ComPtr<IDWriteTextLayout3>      m_textLayout;
		Microsoft::WRL::ComPtr<IDWriteTextFormat2>      m_textFormat;

	};
}


