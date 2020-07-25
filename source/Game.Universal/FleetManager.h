#pragma once

#include "DrawableGameComponent.h"
#include <DirectXMath.h>
#include <vector>
#include "Planet.h"

namespace DirectXGame
{
	class Fleet;
	class Ship;

	class FleetManager final : public DX::DrawableGameComponent
	{
	public:
		/* Constants (defined in source file) */
		static const int32_t NUM_PLANETS;
		static const float_t INTER_PLANET_OFFSET;
		static const float_t HOME_PLANET_RADIUS;

	public:
		FleetManager(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<DX::Camera>& camera);
		
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
		void DrawShips(const Fleet& fleet);
		void DrawDestinationLines(const Fleet& fleet);
		void DrawDrawDirectionLines(const Fleet& fleet);
		
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
		std::vector<std::shared_ptr<Fleet>> mFleets;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
		std::uint32_t mIndexCount;
	};
}



