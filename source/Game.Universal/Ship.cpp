#include "pch.h"
#include "Ship.h"
#include "random.h"

using namespace std;
using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	Ship::Ship(Coord2D destPos, Coord2D startPos, int32_t numOfShipsPerShip, DirectX::XMFLOAT4 color)
	{
		mShipCount = numOfShipsPerShip;

		mDestination = destPos;

		mPosition = startPos;

		mColor = color;

		//get the DrawPoint from CenterPoint
		Coord2D dirVector = mDestination - mPosition;

		// normalize
		dirVector = dirVector / dirVector.magnitude();
		//mVelocity = dirVector;

		mPosition = { startPos.x + (float_t)getRangedRandom(-15, 15), startPos.y + (float_t)getRangedRandom(-15,15) };

		mVelocity = { (float_t)getRangedRandom(-3, 3), (float_t)getRangedRandom(-3,3) };
	}

	bool Ship::checkCollisions(Coord2D position, float_t radius)
	{
		if ((position - mPosition).sqrMagnitude() < pow((radius + mCollisionRadius), 2))
		{
			return true;
		}
		return false;
	}

	void Ship::Update(const StepTimer& timer)
	{
		UNREFERENCED_PARAMETER(timer);

		//mDestination = (InputManagerC::GetInstance()->getCurrentMousePosition());
		if ((mPosition - mDestination).sqrMagnitude() < 225) return;

		mPosition = (mPosition + mVelocity);

		//float_t temp = (mVelocity.normalize()).dot({ 1,0 });

		mOrientationAngle = acosf((mVelocity.normalize()).dot({ 1,0 }));

		mOrientationAngle = toDegrees(mOrientationAngle);
		if (mVelocity.y < 0)
		{
			mOrientationAngle = 360 - mOrientationAngle;
		}
		mOrientationAngle = toRadians(mOrientationAngle);
		mOrientationAngle -= (3 * PI / 4);

		mFrontVertex = { cosf(mOrientationAngle), sinf(mOrientationAngle) };
		mFrontVertex = mPosition + (mFrontVertex * mCollisionRadius);

		mBackLeftVertex = { cosf(mOrientationAngle + (3 * PI / 4)), sinf(mOrientationAngle + (3 * PI / 4)) };
		mBackLeftVertex = mPosition + (mBackLeftVertex * mCollisionRadius);

		mBackRightVertex = { cosf(mOrientationAngle + (3 * PI / 2)), sinf(mOrientationAngle + (3 * PI / 2)) };
		mBackRightVertex = mPosition + (mBackRightVertex * mCollisionRadius);
	}

	//void Ship::render()
	//{
		//const uchar8_t red = static_cast<char8_t>((mColor >> 16) & 0xFF);
		//const uchar8_t green = static_cast<char8_t>((mColor >> 8) & 0xFF);
		//const uchar8_t blue = static_cast<char8_t>((mColor >> 0) & 0xFF);

		//DrawTriangle(mFrontVertex, mBackLeftVertex, mBackRightVertex, red, green, blue);

		//if (StateManagerC::GetInstance()->getMode() != StateManagerC::Mode::DEBUG) return;

		//DrawLine(mPosition.x, mPosition.y, mDestination.x, mDestination.y, 255, 0, 255, 1);
		//DrawLine(mPosition.x, mPosition.y, mPosition.x + (mVelocity.x * 5), mPosition.y + (mVelocity.y * 5), 0, 255, 0, 2);
	//}

	void Ship::flock(std::vector<Ship*> shipList)
	{
		Coord2D flockVector;
		flockVector.x = 0;
		flockVector.y = 0;

		Coord2D sep = flockSeparation(shipList);
		Coord2D coh = flockCohesion(shipList);
		Coord2D ali = flockAlignment(shipList);

		flockVector = flockVector + (sep * separationWeight);
		flockVector = flockVector + (coh * cohesionWeight);
		flockVector = flockVector + (ali * alignmentWeight);

		//flockVector = flockVector  + ((flockSeparation(shipList) * separationWeight));
		//flockVector = flockVector + ((flockCohesion(shipList) * cohesionWeight));
		//flockVector = flockVector + ((flockAlignment(shipList) * alignmentWeight));

		flockVector = flockVector + (((mDestination - mPosition).limit(mMaxForce)) * destinationWeight);

		// normalize
		//flockVector = flockVector / flockVector.magnitude();

		mVelocity = mVelocity + flockVector;

		if (mVelocity.sqrMagnitude() > pow(mMaxSpeed, 2))
		{
			mVelocity = mVelocity.limit(mMaxSpeed);
		}
	}

	Coord2D Ship::flockSeparation(std::vector<Ship*> shipList)
	{
		Coord2D sepVec = { 0,0 };
		int32_t count = 0;

		for (size_t i = 0; i < shipList.size(); i++)
		{
			float_t dist = (shipList.at(i)->mPosition - mPosition).sqrMagnitude();
			if (dist > 0 && dist < pow(mSeparationRadius, 2))
			{
				Coord2D temp = (mPosition - shipList.at(i)->mPosition);
				temp = temp.normalize();
				float_t div = (float_t)(pow(dist, .5f) / mSeparationRadius);

				sepVec = sepVec + (temp / div);
				//sepVec = sepVec + ( (shipList.at(i)->mPosition - mPosition).normalize() / pow(dist, .5f) );
				count++;
			}
		}

		if (count > 0) sepVec = sepVec / (float_t)count;

		return sepVec;
	}

	Coord2D Ship::flockCohesion(std::vector<Ship*> shipList)
	{
		Coord2D cohVec = { 0,0 };
		int32_t count = 0;

		for (size_t i = 0; i < shipList.size(); i++)
		{
			if (shipList.at(i) == this) continue;
			const float_t dist = (shipList.at(i)->mPosition - mPosition).sqrMagnitude();
			if (dist > 0 && dist < pow((double)(mNeighbourRadius), 2))
			{
				cohVec = cohVec + shipList.at(i)->mPosition;
				count++;
			}
		}

		if (count > 0) cohVec = ((cohVec / (float_t)count) - mPosition).limit(mMaxForce);//return ( steerTo(cohVec / count) );

		return cohVec;
	}

	Coord2D Ship::steerTo(Coord2D target)
	{
		Coord2D desired = target - mPosition;
		Coord2D steer = { 0,0 };

		const float_t dist = desired.magnitude();

		if (dist > 0)
		{
			if (dist < 100)
			{
				desired = desired * (mMaxSpeed * (dist / 100.0f));
			}
			else desired = desired * mMaxSpeed;

			steer = desired - mVelocity;

			steer.limit(mMaxForce);
		}

		return steer;
	}

	Coord2D Ship::flockAlignment(std::vector<Ship*> shipList)
	{
		Coord2D aliVec = { 0,0 };
		int32_t count = 0;

		for (size_t i = 0; i < shipList.size(); i++)
		{
			const float_t dist = (shipList.at(i)->mPosition - mPosition).sqrMagnitude();

			if (dist > 0 && dist < pow((double)(mNeighbourRadius), 2))
			{
				aliVec = aliVec + shipList.at(i)->mVelocity;
				count++;
			}
		}

		if (count > 0)
		{
			aliVec = aliVec / (float_t)count;
			aliVec = aliVec.limit(mMaxForce);
		}

		return aliVec;
	}
}