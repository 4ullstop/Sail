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
    cameraResult.position = {-4.2f, 0.04f, 0.77f, 0.0f};

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
    char* boatPath = "../data/obj/boat_V1.obj";
    char* paths = {boatPath};

    
    initData->gameObjs = gameFrameworkCode->GameLoadOBJFiles(platformInfo->parseObjCode,
								      &platformInfo->frameworkArenas,
								      pgMem, memoryPoolCode, &paths, 1);

    v4 spawnObjLoc = v4{0.0f, 0.0f, 0.0f, 1.0f};
#if 0
    gameFrameworkCode->GameSpawnNewOBJ(spawnable_obj_type::sot_ico,
				       spawnObjLoc,
				       &initData->gameObjs,
				       memoryPoolCode);
#else
    transform boatTransform = {};
    boatTransform.location = spawnObjLoc;
    boatTransform.rotation = QuaternionIdentity();
    boatTransform.scale = {1.0f, 1.0f, 1.0f, 1.0f};

    initData->boat = {};    
    initData->boat.objInfo = 
	gameFrameworkCode->GameSpawnNewOBJ(spawnable_obj_type::sot_boat,
					   boatTransform,
					   &initData->gameObjs,
					   memoryPoolCode);



    initData->boat.lerpTimeSpeed = 0.5f;
    initData->boat.currRot =
	initData->boat.startRot = boatTransform.rotation;

#endif    
    return(cameraResult);
}

internal void
RotateBoat(boat_entity* boat, r32 deltaTime, bool32 resetTimer)
{
    if (resetTimer || (boat->currRotTime >= 1.0f))
    {
	boat->currRotTime = 0.0f;
	//reset when button repressed
	boat->startRot = boat->currRot;
    }
    else
    {
	boat->currRotTime += boat->lerpTimeSpeed * deltaTime;
	v4 rotTimeV = {boat->currRotTime, boat->currRotTime, boat->currRotTime, boat->currRotTime};
	//use quaternion rotation before slerp
	boat->currRot = QuaternionSlerpV(boat->startRot, boat->qTargetRot, rotTimeV);
	boat->objInfo->modelTransform.rotation = boat->currRot;
	boat->objInfo->modelMatrix = CreateModelMatrix(boat->objInfo->modelTransform.scale,
						       boat->objInfo->modelTransform.rotation,
						       boat->objInfo->modelTransform.location);
    }
}

extern "C" SAIL_UPDATE(SailUpdate)
{
    //Update our input
    //Update our camera

    game_controller_input* controller = GetController(input, 0);    
#if 0

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

#endif
    if (controller)
    {
	//eventually, it would be nice if the movement was also dependent
	//on the velocity of the boat, meaning we turn more or less depending on how fast the boat
	//is moving or if the boat is moving at all
	v4 rotAdd = {0.0f, 2.0f, 0.0f, 0.0f};

	v4 zAxis = {0.0f, 1.0f, 0.0f, 0.0f};
	bool32 resetTimer = false;
	if (controller->moveLeft.endedDown)
	{
	    initData->boat.qTargetRot = initData->boat.currRot;
	    initData->boat.qTargetRot += QuaternionRotationAxis(zAxis, (r32)RAD2DEG(10));
	    resetTimer = true;
	}

	if (controller->moveRight.endedDown)
	{
	    initData->boat.qTargetRot = initData->boat.currRot;
	    initData->boat.qTargetRot += QuaternionRotationAxis(zAxis, (r32)RAD2DEG(-10));	    
	    resetTimer = true;
	}

	RotateBoat(&initData->boat, deltaTime, resetTimer);
    }
    
    gameFrameworkCode->GameUpdateCamera(camera);
}
