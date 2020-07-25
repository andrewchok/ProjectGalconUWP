#pragma once
#include <stdlib.h>
#include <vector>
#include "Ship.h"
#include "Planet.h"
#include "PlanetManager.h"

namespace DX
{
	class StepTimer;
}

namespace DirectXGame
{
	class Fleet
	{
	public:
		Fleet()
		{
			mNumOfShips = 0;
			mDestination = nullptr;
			mSource = nullptr;
		};
		Fleet(int32_t numOfShips, std::shared_ptr<Planet> destination, std::shared_ptr<Planet> source, DirectX::XMFLOAT4 color);
		~Fleet() {};

		void Update(const DX::StepTimer& timer);

		//void render();

		void shutdown();

		int32_t getNumberOfShipsInFleet();

		void flockFleet();

		std::vector<Ship*> GetShipList() const { return mShipList; };

	private:
		// list of Ships
		std::vector<Ship*> mShipList;

		// actual number of ships, each ship even though rendered to the screen as one could hold multiple ships
		int32_t mNumOfShips;

		// destination
		std::shared_ptr<Planet> mDestination;

		// start position
		std::shared_ptr<Planet> mSource;

		DirectX::XMFLOAT4 mColor;
	};
}
