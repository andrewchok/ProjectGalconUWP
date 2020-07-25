#pragma once

#include <stdlib.h>
#include <vector>
#include <math.h>
#include "Planet.h"
#include "BaseUtils.h"

namespace DX
{
	class StepTimer;
}

namespace DirectXGame
{
	class Ship final
	{
	public:
		Ship(Coord2D destPos, Coord2D startPos, int32_t numOfShipsPerShip, DirectX::XMFLOAT4 color);
		~Ship() {};

		bool checkCollisions(Coord2D position, float_t radius);
		void Update(const DX::StepTimer& timer);


		//void render();
		void flock(std::vector<Ship*> shipList);

		Coord2D flockSeparation(std::vector<Ship*> shipList);
		Coord2D flockCohesion(std::vector<Ship*> shipList);
		Coord2D steerTo(Coord2D target);
		Coord2D flockAlignment(std::vector<Ship*> shipList);

		// negative number for clockwise, positive number for counter-clockwise
		float_t rotateShip(Coord2D destination, Coord2D source)
		{
			return ((source.x * destination.y) - (source.y * destination.x));
		};

		float_t toRadians(float_t angle)
		{
			angle /= 360;
			angle *= 2 * PI;
			return angle;
		};

		float_t toDegrees(float_t angle)
		{
			angle *= 360;
			angle /= 2 * PI;
			return angle;
		};

		const int32_t getShipCount() const { return mShipCount; };
		const DirectX::XMFLOAT4& Color() const { return mColor; };

		const Coord2D& GetFrontVertex() const { return mFrontVertex; };
		const Coord2D& GetBackLeftVertex() const  { return mBackLeftVertex; };
		const Coord2D& GetBackRightVertex() const { return mBackRightVertex; };

		const Coord2D& GetPosition() const { return mPosition; };
		const Coord2D& GetDestination() const { return mDestination; };
		const Coord2D& GetVelocity() const { return mVelocity; };

	private:
		Coord2D mFrontVertex;
		Coord2D mBackLeftVertex;
		Coord2D mBackRightVertex;

		Coord2D mDestination;
		Coord2D mPosition;
		Coord2D mVelocity;
		DirectX::XMFLOAT4 mColor;

		float_t mOrientationAngle;
		float_t mCollisionRadius = 10;
		float_t mNeighbourRadius = 100;
		float_t mSeparationRadius = 20;
		float_t mMaxSpeed = 2;
		float_t mMaxForce = 0.05f;

		int32_t mShipCount;

		const float_t separationWeight = 1;
		const float_t cohesionWeight = 1;
		const float_t alignmentWeight = 1;
		const float_t destinationWeight = 10;
	};
}
