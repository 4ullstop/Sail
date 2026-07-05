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
//    cameraResult.position = {10.0f, 10.f, 10.0f, 10.f};

    cameraResult.movementSpeed = 5.0f;

    r32 aspectX = platformInfo->aspect.x;
    r32 aspectY = platformInfo->aspect.y;
	
//    cameraResult.aspect = {aspectX, aspectY, 0.0f, 0.0f};
    cameraResult.aspect.x = aspectX;
    cameraResult.aspect.y = aspectY;
    cameraResult.aspect.z = 0.0f;
    cameraResult.aspect.w = 0.0f;    


    cameraResult.inheritRotation = true;
    cameraResult.inheritedRotation = QuaternionIdentity();
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
	initData->boat.startRot =
	initData->boat.qTargetRot = boatTransform.rotation;

#endif    
    return(cameraResult);
}

internal void
RotateBoat(boat_entity* boat, r32 deltaTime)
{
    
    boat->isRotating = boat->currRotTime >= 1.0f;
    v4 rotTimeV = {boat->currRotTime, boat->currRotTime, boat->currRotTime, boat->currRotTime};
    
#if 1
    r32 t = 1 - boat->lerpTimeSpeed;
    v4 t4 = {t, t, t, t};
    boat->currRot = QuaternionSlerpV(boat->startRot, boat->qTargetRot, t4);
#else
    boat->currRot = boat->qTargetRot;
#endif	
    boat->objInfo->modelTransform.rotation = boat->currRot;
    boat->objInfo->modelMatrix = CreateModelMatrix(boat->objInfo->modelTransform.scale,
						   boat->objInfo->modelTransform.rotation,
						   boat->objInfo->modelTransform.location);

}

internal void
CalculateNewCameraLocation(game_camera* camera, boat_entity* boat)
{
    v4 camOffset = {-4.2f, 0.04f, 0.77f, 0.0f};	    
    v4 normRot = QuaternionNormalize(boat->currRot);
    v4 invRot = QuaternionConjugate(normRot);
    camera->targetForward = invRot;
    camera->inheritedRotation = invRot;
    camera->position = boat->objInfo->modelTransform.location + Vector3Rotate(camOffset, invRot);    
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
    boat_entity* boat = &initData->boat; 

    if (controller)
    {
	//eventually, it would be nice if the movement was also dependent
	//on the velocity of the boat, meaning we turn more or less depending on how fast the boat
	//is moving or if the boat is moving at all

	i32 deg = 1; // * deltaTime ??
	v4 zAxis = {0.0f, 1.0f, 0.0f, 0.0f};

	v4 camOffset = {-4.2f, 0.04f, 0.77f, 0.0f};	
	camera->inheritedOffset = camOffset;
	if (controller->moveLeft.endedDown)
	{
	    boat->currRotTime += boat->lerpTimeSpeed * deltaTime;	    
	    boat->qTargetRot = QuaternionNormalize(boat->qTargetRot);	    
	    boat->qTargetRot = QuaternionMultiply(boat->qTargetRot,
						  QuaternionRotationAxis(zAxis, (r32)DEG2RAD(deg)));


	    RotateBoat(&initData->boat, deltaTime);

	    CalculateNewCameraLocation(camera, boat);
	}

	if (controller->moveRight.endedDown)
	{
	    boat->qTargetRot = QuaternionNormalize(boat->qTargetRot);
	    boat->qTargetRot = QuaternionMultiply(boat->qTargetRot,
						  QuaternionRotationAxis(zAxis, (r32)DEG2RAD(-deg))); 


	    RotateBoat(&initData->boat, deltaTime);

	    CalculateNewCameraLocation(camera, boat);
	}

    }
    

    gameFrameworkCode->GameUpdateCamera(camera);


}
