#include "sail_data_collection.h"


internal void
RunPlayerTutorial(game_state* gameState, sailing* sailInfo)
{
    switch(gameState->tutorialData.tutorialState)
    {
    case ts_sideToSide:
    {
	if (gameState->tutorialData.movedToSides)
	    gameState->tutorialData.tutorialState = ts_upToSpeed;

    } break;
    case ts_upToSpeed:
    {
	if (sailInfo->windRotation.windInRotation) return;
	if (gameState->tutorialData.upToSpeed)
	    gameState->tutorialData.timeSpentInState += gameState->msPerFrame / 1000.0f;	    

	if (gameState->tutorialData.timeSpentInState >= 5.0f)
	{
	    gameState->tutorialData.tutorialState = ts_windChange;
	    gameState->tutorialData.windChanged = false;
	}
	
    } break;
    case ts_windChange:
    {
	if (!gameState->tutorialData.windChanged)
	{
	    r32 randomRotation = RandomNumberBetween(15.0f, 90.0f);
	    sailInfo->windRotation.eTargetRotations = AddTargetYawRotation(randomRotation,
									   sailInfo->windRotation.eTargetRotations);

	    gameState->tutorialData.windChangeAmount++;
	    gameState->tutorialData.windChanged = true;
	    gameState->tutorialData.timeSpentInState = 0.0f;
	    if (gameState->tutorialData.windChangeAmount >= 3)
		gameState->tutorialData.tutorialState = ts_storm;
	    else
	    {
		gameState->tutorialData.tutorialState = ts_upToSpeed;
	    }
	    
	}
    } break;
    case ts_storm:
    {
	//Wreck havoc
    } break;
    default:
    {
	
    } break;
    }
}
