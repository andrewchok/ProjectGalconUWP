#pragma once
#include "Fleet.h"

#include "Planet.h"


namespace DirectXGame
{
	class Player
	{
	public:
		Player(const char8_t playerID);
		~Player() {};

		void Update(const DX::StepTimer& timer);

		void shutdown();

		const char8_t getPlayerID();
		void setColor(const DirectX::XMFLOAT4& color);
		const DirectX::XMFLOAT4& Color() const;

		// get, set current/selectedPlanets
		void setHighlightedPlanet(std::shared_ptr<Planet> planet);
		void setHighlightedPlanet(Planet::Neighbor neighbor);
		void selectPlanet();

		std::shared_ptr<Planet> GetHighlightedPlanet() const;
		std::shared_ptr<Planet> GetSelectedPlanet() const;

		std::vector<Fleet*> GetFleetList() const;

		int32_t GetPecentageSelection();

		void changePercentageSelection();

		// get number of ships that are enroute
		int32_t getNumberOfTravellingShips();

		void sendFleet(int32_t numOfShips);
	private:
		char8_t mPlayerID;
		DirectX::XMFLOAT4 mColor;

		std::shared_ptr<Planet> mHighlightedPlanet = nullptr;
		std::shared_ptr<Planet> mSelectedPlanet = nullptr;

		// array of fleets
		std::vector<Fleet*> mFleetList;

		// value [0..100]
		int32_t mPercentageSelection;

		//std::vector<PlanetC*> mOwnedPlanetList;
	};
}

