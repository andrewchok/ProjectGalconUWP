#include "pch.h"
#include "StateManager.h"

StateManager* StateManager::sInstance = nullptr;


StateManager* StateManager::GetInstance()
{
	if (sInstance == nullptr) sInstance = new StateManager();
	return sInstance;
}

void StateManager::init()
{
	mCurrentState = State::MENU;
	mCurrentMode = Mode::GAME;
	mCurrentWinner = Winner::INVAL_WINNER;
}

void StateManager::update()
{
	
}

StateManager::State StateManager::getState() const
{
	return mCurrentState;
}

void StateManager::setState(State state)
{
	mCurrentState = state;
}
StateManager::Mode StateManager::getMode() const
{
	return mCurrentMode;
}

void StateManager::setMode(Mode mode)
{
	mCurrentMode = mode;
}

StateManager::Winner StateManager::getWinner() const
{
	return mCurrentWinner;
}

void StateManager::setWinner(Winner player)
{
	mCurrentWinner = player;
}

void StateManager::shutdown()
{
}