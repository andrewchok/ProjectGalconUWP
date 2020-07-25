#include "pch.h"
#include "Fleet.h"
#include "Fleet.h"

using namespace std;
using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	Fleet::Fleet(int32_t numOfShips, std::shared_ptr<Planet> destination, std::shared_ptr<Planet> source, DirectX::XMFLOAT4 color)
	{
		// if we want to scale the ships on screen to represent more than just one ship
		const int32_t capOfShipsToSpawn = 50;
		const int32_t numOfShipsPerShip = 1 + (int32_t)(numOfShips / capOfShipsToSpawn);

		mNumOfShips = numOfShips;
		mDestination = destination;
		mSource = source;
		mColor = color;

		for (int i = 0; i < (mNumOfShips / numOfShipsPerShip); i++)
		{
			mShipList.push_back(new Ship(destination->GetPlanetPosition(), source->GetPlanetPosition(), numOfShipsPerShip, mColor));
		}

		// any remainder is added
		if (mNumOfShips % numOfShipsPerShip > 0)
		{
			mShipList.push_back(new Ship(destination->GetPlanetPosition(), source->GetPlanetPosition(), mNumOfShips % numOfShipsPerShip, mColor));
		}
	}



	void Fleet::Update(const StepTimer& timer)
	{
		// deletes one ship each update that is colliding
		for (size_t i = 0; i < mShipList.size(); i++)
		{
			if (mShipList.at(i)->checkCollisions(mDestination->GetPlanetPosition(), mDestination->Radius()))
			{
				if (CompareColors(mDestination->Color(), mColor))
				{
					mDestination->SetShipCount(mDestination->ShipCount() + mShipList.at(i)->getShipCount());
				}
				else
				{
					for (int j = 0; j < mShipList.at(i)->getShipCount(); j++)
					{
						if (CompareColors(mDestination->Color(), mColor))
						{
							mDestination->SetShipCount(mDestination->ShipCount() + mShipList.at(i)->getShipCount());
						}
						else if (mDestination->ShipCount() < 0)
						{
							mDestination->SetColor(mColor);
							mDestination->SetShipCount(1);
						}
						else mDestination->SetShipCount(mDestination->ShipCount() - 1);
					}
				}

				mNumOfShips -= mShipList.at(i)->getShipCount();
				delete(mShipList.at(i));
				mShipList.erase(mShipList.begin() + i);
				break;
			}
		}

		flockFleet();
		for (size_t i = 0; i < mShipList.size(); i++)
		{
			mShipList.at(i)->Update(timer);
		}
	}

	//void Fleet::render()
	//{
	//	for (size_t i = 0; i < mShipList.size(); i++)
	//	{
	//		mShipList.at(i)->render();
	//	}
	//}

	void Fleet::shutdown()
	{
		for (size_t i = mShipList.size() - 1; i >= 0; i--)
		{
			delete(mShipList.at(i));
			mShipList.erase(mShipList.begin() + i);
		}
		mShipList.clear();
	}

	int32_t Fleet::getNumberOfShipsInFleet()
	{
		return mNumOfShips;
	}

	void Fleet::flockFleet()
	{
		for (size_t i = 0; i < mShipList.size(); i++)
		{
			mShipList.at(i)->flock(mShipList);
		}
	}
}