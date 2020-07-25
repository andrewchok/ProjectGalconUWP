#pragma once

#include "Transform2D.h"
#include "VectorHelper.h"
#include <DirectXMath.h>
#include <DirectXColors.h>
#include "BaseUtils.h"

namespace DX
{
	class StepTimer;
}

namespace DirectXGame
{
	class PlanetManager;

	/* Structures */

	enum struct Neighbor
	{
		INVAL_DIRECTION = -1,

		LEFT,
		RIGHT,
		UP,
		DOWN,

		MAX_DIRECTION
	};

	class Planet final
	{
	public:
		/* Constants */

		static const float_t BORDER_OFFSET;
		static const float_t MIN_RADIUS;
		static const float_t MAX_RADIUS;
		static const DirectX::XMFLOAT4 PLANET_DEFAULT_COLOR;

		static const float_t PRODUCTION_MIN;
		static const float_t PRODUCTION_MAX;
		static const float_t MIN_DEFAULT_SHIPS;
		static const float_t MAX_DEFAULT_SHIPS;

		/* Structures */

		enum struct Neighbor
		{
			INVAL_DIRECTION = -1,

			LEFT,
			RIGHT,
			UP,
			DOWN,

			MAX_DIRECTION
		};

	public:
		Planet(PlanetManager& planetManager);
		Planet(PlanetManager& PlanetManager, const DX::Transform2D& transform, float radius, const DirectX::XMFLOAT4& color = PLANET_DEFAULT_COLOR);
		Planet(const Planet&) = default;
		Planet& operator=(const Planet&) = delete;
		Planet(Planet&&) = default;
		Planet& operator=(Planet&&) = default;
		~Planet() = default;

		const DX::Transform2D& Transform() const;
		void SetTransform(const DX::Transform2D& transform);

		const Coord2D GetPlanetPosition() const;
		void SetPlanetPosition(const Coord2D& position);

		float Radius() const;
		void SetRadius(const float radius);

		const DirectX::XMFLOAT4& Color() const;
		void SetColor(const DirectX::XMFLOAT4& color);
				
		void Update(const DX::StepTimer& timer);

		/* Planet Things */

		void initRandom();
		
		const std::shared_ptr<Planet> GetNeighbor(Neighbor direction) const;
		void SetNeighbors(std::shared_ptr<Planet>& left, std::shared_ptr<Planet>& right, std::shared_ptr<Planet>& up, std::shared_ptr<Planet>& down);

		float_t ShipCount() const;
		void SetShipCount(const float_t shipCount);

	private:

		//BallManager& mBallManager;
		DX::Transform2D mTransform;
		float mRadius;
		DirectX::XMFLOAT4 mColor;

		/* Planet Vars */

		PlanetManager& mPlanetManager;

		struct
		{
			std::shared_ptr<Planet> left = nullptr;
			std::shared_ptr<Planet> right = nullptr;
			std::shared_ptr<Planet> up = nullptr;
			std::shared_ptr<Planet> down = nullptr;
		} mNeighbors;

		float_t mProductionRate = 0;
		float_t mShipCount = 0;
	};
}

