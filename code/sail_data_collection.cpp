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
	if (gameState->tutorialData.upToSpeed)
	    gameState->tutorialData.timeSpentInState += gameState->msPerFrame / 1000.0f;	    

	if (gameState->tutorialData.timeSpentInState >= 5.0f)
	    gameState->tutorialData.tutorialState = ts_windChange;
    } break;
    case ts_windChange:
    {
	if (!gameState->tutorialData.windChanged)
	{
	    AddTargetWindRotation(45.0f, &sailInfo->windRotation);
	    gameState->tutorialData.windChanged = true;
	}
    } break;
    default:
    {
	
    } break;
    }
}
