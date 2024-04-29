#include "pch.h"
#include "Player.h"
#include "Fleet.h"

using namespace std;
using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	/* Methods */

	Player::Player(const char8_t playerID)
	{
		mPlayerID = playerID;
		mPercentageSelection = 50;
	}

	int32_t Player::getNumberOfTravellingShips()
	{
		int32_t numOfShips = 0;


		// linker error
		for (size_t i = 0; i < mFleetList.size(); i++)
		{
			numOfShips += mFleetList.at(i)->getNumberOfShipsInFleet();
		}

		return numOfShips;
	}

	void Player::sendFleet(int32_t numOfShips)
	{
		mFleetList.push_back(new Fleet(numOfShips, mHighlightedPlanet, mSelectedPlanet, mColor));
	}

	const char8_t Player::getPlayerID() const
	{
		return mPlayerID;
	}

	void Player::setColor(const DirectX::XMFLOAT4& color)
	{
		mColor = color;
	}

	const DirectX::XMFLOAT4& Player::Color() const
	{
		return mColor;
	}

	void Player::setHighlightedPlanet(std::shared_ptr<Planet> planet)
	{
		mHighlightedPlanet = planet;
	}

	void Player::setHighlightedPlanet(Planet::Neighbor neighbor)
	{
		std::shared_ptr<Planet> tmp = mHighlightedPlanet->GetNeighbor(neighbor);
		if (tmp) mHighlightedPlanet = tmp;
	}

	void Player::selectPlanet()
	{
		if (!mSelectedPlanet && mHighlightedPlanet && CompareColors(mHighlightedPlanet->Color(), mColor))
		{
			mSelectedPlanet = mHighlightedPlanet;
		}
		else if (mSelectedPlanet && mSelectedPlanet == mHighlightedPlanet)
		{
			mSelectedPlanet = nullptr;
		}
		else if (mSelectedPlanet && mHighlightedPlanet)
		{
			int32_t shipsToSend = (int32_t)(mSelectedPlanet->ShipCount() * (((float_t)mPercentageSelection) / 100.0f));
			sendFleet(shipsToSend);
			mSelectedPlanet->SetShipCount(mSelectedPlanet->ShipCount() - shipsToSend);
			mSelectedPlanet = nullptr;
		}
	}

	std::shared_ptr<Planet> Player::GetHighlightedPlanet() const
	{
		return mHighlightedPlanet;
	}

	std::shared_ptr<Planet> Player::GetSelectedPlanet() const
	{
		return mSelectedPlanet;
	}

	std::vector<Fleet*> Player::GetFleetList() const
	{
		return mFleetList;
	}

	int32_t Player::GetPecentageSelection()
	{
		return mPercentageSelection;
	}

	void Player::changePercentageSelection()
	{
		mPercentageSelection += 25;
		if (mPercentageSelection > 100) mPercentageSelection = 25;
	}

	void Player::Update(const StepTimer& timer)
	{
		//double elapsedTime = timer.GetElapsedSeconds();

		if (mSelectedPlanet != nullptr)
		{
			if (!CompareColors(mSelectedPlanet->Color(), mColor)) mSelectedPlanet = nullptr;
		}
		
		for (size_t i = 0; i < mFleetList.size(); i++)
		{
			mFleetList.at(i)->Update(timer);
		}
		
	}

	void Player::shutdown()
	{
		for (int32_t i = static_cast<int32_t>(mFleetList.size()) - 1; i >= 0; i--)
		{
			mFleetList.at(i)->shutdown();
			delete(mFleetList.at(i));
			mFleetList.pop_back();
		}
	}
}