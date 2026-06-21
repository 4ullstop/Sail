#include "sail_game_layer.h"

extern "C" SAIL_INITIALIZE(SailInitialize)
{


    game_camera cameraResult = {};
    
    cameraResult.startEye = {0.0f, 0.7f, 1.5f, 0.f};
    cameraResult.startAt = {0.0f, -0.1f, 0.0f, 0.f};
    cameraResult.startUp = {0.0f, 1.0f, 0.0f, 0.f};

    cameraResult.yaw = -90.0f;
    cameraResult.pitch = 0.0f;
    cameraResult.front = {0.0f, 0.0f, -1.0f, 0.0f};
    cameraResult.position = {10.0f, 4.0f, 9.0f, 0.0f};

    cameraResult.movementSpeed = 5.0f;

    r32 aspectX = platformInfo->aspect.x;
    r32 aspectY = platformInfo->aspect.y;
	
//    cameraResult.aspect = {aspectX, aspectY, 0.0f, 0.0f};
    cameraResult.aspect.x = aspectX;
    cameraResult.aspect.y = aspectY;
    cameraResult.aspect.z = 0.0f;
    cameraResult.aspect.w = 0.0f;    
    
    gameFrameworkCode->GameCreateViewAndPerspective(&cameraResult);

    size_t objectArenaAllocSize = Megabytes(10);

    platformInfo->frameworkArenas.spawnedObjectArena =
	(memory_arena*)memoryPoolCode->PushStruct(platformInfo->frameworkArenas.setupArena, sizeof(memory_arena));
    
    memoryPoolCode->InitArena(platformInfo->frameworkArenas.spawnedObjectArena,
			      objectArenaAllocSize,
			      pgMem,
			      e_arena_type::permanent);



    char* icoPath = "../data/obj/debug_ico.obj";
    char* paths = {icoPath};

    
    initData->gameObjs = gameFrameworkCode->GameLoadOBJFiles(platformInfo->parseObjCode,
								      &platformInfo->frameworkArenas,
								      pgMem, memoryPoolCode, &paths, 1);

    v4 spawnObjLoc = v4{0.0f, 0.0f, 0.0f, 0.0f};
    gameFrameworkCode->GameSpawnNewOBJ(spawnable_obj_type::sot_ico,
				       spawnObjLoc,
				       &initData->gameObjs,
				       memoryPoolCode);
    
    return(cameraResult);
}

extern "C" SAIL_UPDATE(SailUpdate)
{
    //Update our input
    //Update our camera

    game_controller_input* controller = GetController(input, 0);
    r32 velocity = camera->movementSpeed * deltaTime;
    if (controller)
    {
	if (controller->moveForward.endedDown)
	{
	    //w
	    camera->position = camera->position + (camera->front * velocity);
	}

	if (controller->moveLeft.endedDown)
	{
	    //a
	    camera->position = camera->position - (camera->right * velocity);
	}

	if (controller->moveBackward.endedDown)
	{
	    //s
	    camera->position = camera->position - (camera->front * velocity);	    
	}

	if (controller->moveRight.endedDown)
	{
	    //d
	    camera->position = camera->position + (camera->right * velocity);
	}
    }
    gameFrameworkCode->GameUpdateCamera(camera);
}
