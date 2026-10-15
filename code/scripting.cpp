#include "scripting.h"

internal void
PlayerMovedToSide(game_state* gameState)
{
    hud* gameHUD = gameState->gameHUD;

    gameHUD->rightBumper.uiInfo->hidden = true;
    gameHUD->leftBumper.uiInfo->hidden = true;

    if (gameState->currentCamLocation == scl_left)
    {
	gameHUD->leftStick.uiInfo->hidden = false;
    }

    if (gameState->currentCamLocation == scl_right)
    {
	gameHUD->rightStick.uiInfo->hidden = false;
    }
    gameState->tutorialData.uiState = us_sticks;
}
/*
  Tell player to move to the left -> have them rotate sail to correct position to catch wind
  -> move the wind -> tell the player to move to right side -> tell player to rotate sail to correct position
 */

internal void
ScriptTutorial(tutorial_data* data, game_state* gameState, boat_entity* boat)
{
    hud* gameHUD = gameState->gameHUD;
    switch(data->tutorialUIState)
    {
    case tus_start:
    {
	data->tutorialUIState = tus_moveLeft;
    } break;
    case tus_moveLeft:
    {
	if (gameState->currentCamLocation == scl_left)
	{
	    gameHUD->leftBumper.uiInfo->hidden = true;
	    data->tutorialUIState = tus_rotateClockwise;
	}
	else 
	{
	    if (gameHUD->leftBumper.uiInfo->hidden)
	    {
		gameHUD->leftBumper.uiInfo->hidden = false;
	    }
	}

    } break;
    case tus_rotateClockwise:
    {
	if (gameHUD->leftStick.uiInfo->hidden)
	{
	    gameHUD->leftStick.uiInfo->hidden = false;
	}
	else
	{
	    if (boat->movementSpeed > 3.0f)
	    {
		gameHUD->leftStick.uiInfo->hidden = true;
		data->tutorialUIState = tus_moveRight;
		gameHUD->rightBumper.uiInfo->hidden = false;
		RotateWindDirection(&boat->sailInfo, 45.0f);
		data->windChanged = true;
	    }
	}
    } break;
    case tus_moveRight:
    {
	if (gameState->currentCamLocation == scl_right)
	{
	    gameHUD->rightBumper.uiInfo->hidden = true;

	    data->tutorialUIState = tus_rotateCounterClockwise;
	    
	}
	else
	{
	    if(gameHUD->rightBumper.uiInfo->hidden)
	    {
		gameHUD->rightBumper.uiInfo->hidden = false;
	    }
	}
    } break;
    case tus_rotateCounterClockwise:
    {
	if (gameHUD->rightStick.uiInfo->hidden)
	{
	    gameHUD->rightStick.uiInfo->hidden = false;
	}
	else
	{
	    bool32 inRotation = !(IsInRange(boat->sailInfo.windRotation.eTargetRotations.yaw - 1.0f,
					   boat->sailInfo.windRotation.eTargetRotations.yaw + 1.0f,
					   boat->sailInfo.windRotation.eCurrentRotations.yaw));
	    if (boat->movementSpeed >= 4.8f && !inRotation)
	    {
		gameHUD->rightStick.uiInfo->hidden = true;
		data->tutorialUIState = tus_complete;
	    }
	}
    } break;
    case tus_complete:
    {
	data->uiTutorialComplete = true;
    };
    default:
    {

    } break;
    }
}
