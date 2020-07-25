#include "pch.h"
#include "Planet.h"
#include "OrthographicCamera.h"

using namespace std;
using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	/* Constants */

	const float_t Planet::BORDER_OFFSET = 120;
	const float_t Planet::MIN_RADIUS = 15;
	const float_t Planet::MAX_RADIUS = 60;
	const DirectX::XMFLOAT4 Planet::PLANET_DEFAULT_COLOR = XMFLOAT4(0.3137f, 0.3137f, 0.3137f, 1.0f);

	const float_t Planet::PRODUCTION_MIN = 0.2f;
	const float_t Planet::PRODUCTION_MAX = 1.5f;
	const float_t Planet::MIN_DEFAULT_SHIPS = 1;
	const float_t Planet::MAX_DEFAULT_SHIPS = 50;

	Planet::Planet(PlanetManager& planetManager) :
		mPlanetManager(planetManager)
	{
		mColor = PLANET_DEFAULT_COLOR;

		mTransform = { XMFLOAT2(0, 0 ), 0};

		mRadius = 10;
	}

	Planet::Planet(PlanetManager& planetManager, const DX::Transform2D& transform, float radius, const DirectX::XMFLOAT4& color) :
		mPlanetManager(planetManager), mTransform(transform), mRadius(radius),
		mColor(color)
	{
		mProductionRate = (((mRadius - MIN_RADIUS) / (MAX_RADIUS - MIN_RADIUS)) * PRODUCTION_MAX) + PRODUCTION_MIN;
	}

	const Transform2D& Planet::Transform() const
	{
		return mTransform;
	}

	void Planet::SetTransform(const Transform2D& transform)
	{
		mTransform = transform;
	}

	const Coord2D Planet::GetPlanetPosition() const
	{
		return Coord2D(mTransform.Position().x, mTransform.Position().y);
	}

	void Planet::SetPlanetPosition(const Coord2D& position)
	{
		SetTransform(XMFLOAT2(position.x, position.y));
	}

	float Planet::Radius() const
	{
		return mRadius;
	}

	void Planet::SetRadius(const float radius)
	{
		mRadius = radius;
		mProductionRate = (((mRadius - MIN_RADIUS) / (MAX_RADIUS - MIN_RADIUS)) * PRODUCTION_MAX) + PRODUCTION_MIN;
	}

	const XMFLOAT4& Planet::Color() const
	{
		return mColor;
	}

	void Planet::SetColor(const XMFLOAT4& color)
	{
		mColor = color;
	}
	
	void Planet::Update(const StepTimer& timer)
	{
		double elapsedTime = timer.GetElapsedSeconds();

		if (mColor.x != PLANET_DEFAULT_COLOR.x && mColor.y != PLANET_DEFAULT_COLOR.y && mColor.z != PLANET_DEFAULT_COLOR.z)
		{
			mShipCount += mProductionRate * static_cast<float>(elapsedTime);
		}
	}

	void Planet::initRandom()
	{
		random_device device;
		default_random_engine generator(device());

		const float_t maxX = OrthographicCamera::DefaultViewWidth - BORDER_OFFSET;
		const float_t maxY = OrthographicCamera::DefaultViewHeight - BORDER_OFFSET;

		uniform_real_distribution<float> xDistribution(BORDER_OFFSET, maxX);
		uniform_real_distribution<float> yDistribution(BORDER_OFFSET, maxY);
		uniform_real_distribution<float> radiusDistribution(MIN_RADIUS, MAX_RADIUS);
		uniform_real_distribution<float> lowShipDistribution(MIN_DEFAULT_SHIPS, MAX_DEFAULT_SHIPS/4);
		uniform_real_distribution<float> midShipDistribution(MAX_DEFAULT_SHIPS / 4, MAX_DEFAULT_SHIPS/4 * 3);
		uniform_real_distribution<float> highShipDistribution(MAX_DEFAULT_SHIPS / 4 * 3, MAX_DEFAULT_SHIPS);
		uniform_real_distribution<float> shipRatioDistribution(0, 1);

		mTransform = 
		{ 
			/*Position: */	XMFLOAT2(xDistribution(generator), yDistribution(generator)),
			/*Rotation: */	0
		};
		mRadius = radiusDistribution(generator);//getRangedRandom(MIN_RADIUS, MAX_RADIUS);
		mProductionRate = (((mRadius - MIN_RADIUS) / (MAX_RADIUS - MIN_RADIUS)) * PRODUCTION_MAX) + PRODUCTION_MIN;

		const float_t random = shipRatioDistribution(generator);
		if (random < 0.2)
			mShipCount = highShipDistribution(generator);
		else if (random < 0.5)
			mShipCount = lowShipDistribution(generator);
		else
			mShipCount = midShipDistribution(generator);
	}

	const std::shared_ptr<Planet> Planet::GetNeighbor(Neighbor direction) const
	{
		std::shared_ptr<Planet> neighbor = nullptr;

		switch (direction)
		{
		case Neighbor::LEFT:
			neighbor = mNeighbors.left;
			break;
		case Neighbor::RIGHT:
			neighbor = mNeighbors.right;
			break;
		case Neighbor::UP:
			neighbor = mNeighbors.up;
			break;
		case Neighbor::DOWN:
			neighbor = mNeighbors.down;
			break;

		default:
			break;
		}

		return neighbor;
	}

	void Planet::SetNeighbors(std::shared_ptr<Planet>& left, std::shared_ptr<Planet>& right, std::shared_ptr<Planet>& up, std::shared_ptr<Planet>& down)
	{
		mNeighbors.left = left;
		mNeighbors.right = right;
		mNeighbors.up = up;
		mNeighbors.down = down;
	}

	float_t Planet::ShipCount() const
	{
		return mShipCount;
	}

	void Planet::SetShipCount(const float_t shipCount) 
	{
		mShipCount = shipCount;
	}
}