#include "sail_game_layer.h"

//Sailing mechanic

internal v4
GetForwardFromQuat(v4 inQuat, r32* pitch, r32* yaw)
{
    v4 quat = QuaternionNormalize(inQuat);
    quat = QuaternionConjugate(quat);
    m4 m = MatrixRotationQuaternion(quat);
    r32 forwardX = m.e[2][0];
    r32 forwardZ = m.e[2][2];

    *yaw = atan2f(forwardX, forwardZ);
    v4 result = GetForwardVector(*pitch, *yaw);
    
    return(result);
}

internal void
UpdateBoatVectors(boat_entity* boat)
{
    boat->forward = GetForwardFromQuat(boat->currRot, &boat->pitch, &boat->yaw);
    sail_type* main = &boat->sailInfo.mainSail;
    main->sailForward = GetForwardFromQuat(main->qSailRot, &main->pitch, &main->yaw);
    //Now that we have the forwards of both of the objects we can compare against the boat and the wind direction

    //boat vs wind dir
    //sail vs boat
}

internal void
UpdateSailOrientations(boat_entity* boat)
{
//now we just have to do the calculations like how we did with the camera just with the sails
    //to get our orientation and then use it to compare against the boat,
    //once we get this angle in combination with getting the angle of the boat and the wind direction,
    //there should be a way to compare everything and put it into a number for the purposes of calculating
    //the speed of the boat
}


internal inherited_location_info
CalculateNewInheritedLocation(v4 offset, v4 rotation, v4 location)
{
    inherited_location_info result = {};
    v4 normRot = QuaternionNormalize(rotation);
    v4 outRot = QuaternionConjugate(normRot);
    result.targetForward = normRot;
    result.inheritedRotation = normRot;
    result.position = location + Vector3Rotate(offset, normRot);
    return(result);
}

internal void
CalculateCameraLocation(game_camera* camera, boat_entity* boat)
{

    inherited_location_info locInfo =
	CalculateNewInheritedLocation(boat->currCamOffset,
				      boat->currRot,
				      boat->objInfo->modelTransform.location);
    camera->targetForward = locInfo.targetForward;
    camera->inheritedRotation = locInfo.inheritedRotation;
    camera->position = locInfo.position;    
}


//Temporary for testing our sail orientation
//Eventually this will only be according to the inputs of winding the winches
internal void
ProcessSailInputs(game_controller_input* controller, boat_entity* boat)
{
    sail_type* main = &boat->sailInfo.mainSail;
    if (controller->one.started)
    {
	main->sailOrientation = so_closeHauled;
	controller->one.started = false;	
    }
    if (controller->two.started)
    {
	main->sailOrientation = so_closeReach;	
	controller->two.started = false;	
    }
    if (controller->three.started)
    {
	main->sailOrientation = so_beamReach;	
	controller->three.started = false;	
    }
    if (controller->four.started)
    {
	main->sailOrientation = so_broadReach;	
	controller->four.started = false;	
    }
    if (controller->five.started)
    {
	main->sailOrientation = so_running;	
	controller->five.started = false;	
    }
}

//External calls

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
    char* mastPath = "../data/obj/boat_V1_mast.obj";
    char* paths[256] = {boatPath, mastPath};

    
    initData->gameObjs = gameFrameworkCode->GameLoadOBJFiles(platformInfo->parseObjCode,
								      &platformInfo->frameworkArenas,
								      pgMem, memoryPoolCode, paths, 2);

    v4 spawnObjLoc = v4{0.0f, 0.0f, 10.0f, 1.0f};
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
					   memoryPoolCode,
					   false,
					   Identity());

    //MAIN SAIL
    v4 mastLocation = {0.0f, -0.7f, 0.65f, 1.0f};
    initData->boat.sailInfo = {};    
    initData->boat.sailInfo.mainSail.locationOffset = mastLocation;
    initData->boat.sailInfo.mainSail.startRot = 
	initData->boat.sailInfo.mainSail.qTargetRot = QuaternionIdentity();
    transform mastTransform = {};
    mastTransform.location = mastLocation;
    mastTransform.rotation = QuaternionIdentity();
    mastTransform.scale = {1.0f, 1.0f, 1.0f, 1.0f};
    initData->boat.mast =
	gameFrameworkCode->GameSpawnNewOBJ(spawnable_obj_type::sot_mast,
					   mastTransform,
					   &initData->gameObjs,
					   memoryPoolCode,
					   true,
					   initData->boat.objInfo->modelMatrix);

    

    //BOAT
    v4 camOffset = {-4.2f, 0.04f, 0.77f, 0.0f};

    initData->boat.lerpTimeSpeed = 1.f;
    initData->boat.currRot =
	initData->boat.startRot =
	initData->boat.qTargetRot = boatTransform.rotation;

    initData->boat.sailInfo.mainSail.qSailRot =
	initData->boat.sailInfo.mainSail.startRot =
	initData->boat.sailInfo.mainSail.qTargetRot = boatTransform.rotation;
    
    initData->boat.staticCamLocation = static_cam_location::scl_center;
    initData->boat.currCamOffset =
	initData->boat.centerCamOffset = camOffset;
    initData->boat.leftCamOffset = {camOffset.x, camOffset.y, camOffset.z - 1.0f, camOffset.w};
    initData->boat.rightCamOffset = {camOffset.x, camOffset.y, camOffset.z + 1.0f, camOffset.w};

    CalculateCameraLocation(&cameraResult, &initData->boat);
#endif    
    return(cameraResult);
}

internal void
ChangeCamOffset(static_cam_location newCamLocation, boat_entity* boat)
{
    if (newCamLocation == boat->staticCamLocation)
	return;
    switch (boat->staticCamLocation)
    {
    case static_cam_location::scl_center:
    {
	if (newCamLocation == static_cam_location::scl_left)
	{
	    boat->currCamOffset = boat->leftCamOffset;
	    boat->staticCamLocation = static_cam_location::scl_left;
	}
	else if (newCamLocation == static_cam_location::scl_right)
	{
	    boat->currCamOffset = boat->rightCamOffset;
	    boat->staticCamLocation = static_cam_location::scl_right;
	}
    } break;
    case static_cam_location::scl_left:
    {
	if (newCamLocation == static_cam_location::scl_right)
	{
	    boat->currCamOffset = boat->centerCamOffset;
	    boat->staticCamLocation = static_cam_location::scl_center;
	}
    } break;
    case static_cam_location::scl_right:
    {
	if (newCamLocation == static_cam_location::scl_left)
	{
	    boat->currCamOffset = boat->centerCamOffset;
	    boat->staticCamLocation = static_cam_location::scl_center;
	}
    } break;
    };
}



internal v4 
RotateOBJ(spawned_obj_info* objInfo, r32 deltaTime, v4 startRot, v4* targetRot, r32 lerpSpeed, v4 axis, r32 degrees, m4 parentTransform)
{
    *targetRot = QuaternionNormalize(*targetRot);
    *targetRot = QuaternionMultiply(*targetRot,
				    QuaternionRotationAxis(axis, (r32)DEG2RAD(degrees)));

    
    r32 t = 1 - lerpSpeed * deltaTime;
    v4 t4 = {t, t, t, t};
    v4 outRotation = QuaternionSlerpV(startRot, *targetRot, t4);

    if (objInfo)
    {
	objInfo->modelTransform.rotation = outRotation;
	if (objInfo->inheritsTransform)
	{
	    objInfo->localMatrix = CreateModelMatrix(objInfo->modelTransform.scale,
						     objInfo->modelTransform.rotation,
						     objInfo->modelTransform.location);
	    objInfo->modelMatrix = objInfo->localMatrix * parentTransform;
	}
	else
	{
	    objInfo->modelMatrix = CreateModelMatrix(objInfo->modelTransform.scale,
						     objInfo->modelTransform.rotation,
						     objInfo->modelTransform.location);
	}
    }
    
    
    return(outRotation);
}

internal v4
RotateOBJ(spawned_obj_info* objInfo, r32 deltaTime, v4 startRot, v4* targetRot, r32 lerpSpeed, v4 axis, r32 degrees)
{
    return(RotateOBJ(objInfo, deltaTime, startRot, targetRot, lerpSpeed, axis, degrees, Identity()));
}


internal void
CalculateMastLocation(sail_type* main, boat_entity* boat)
{

#if 0    
    inherited_location_info locInfo =
	CalculateNewInheritedLocation(main->locationOffset,
				   main->qSailRot,
				   boat->mast->modelTransform.location);

    boat->mast->modelTransform.location = locInfo.position;
    main->qSailRot = locInfo.inheritedRotation;
#else
    boat->mast->modelMatrix = boat->mast->localMatrix * boat->objInfo->modelMatrix;
#endif    
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
	//Q
	if (controller->moveDown.started)
	{
	    ChangeCamOffset(static_cam_location::scl_left, boat);
	    CalculateCameraLocation(camera, boat);
	    controller->moveDown.started = false;
	}

	//E
	if (controller->moveUp.started)
	{
	    ChangeCamOffset(static_cam_location::scl_right, boat);
	    CalculateCameraLocation(camera, boat);
	    controller->moveUp.started = false;
	}

	if ((boat->staticCamLocation == scl_right) || (boat->staticCamLocation == scl_left))
	{
	    boat->boatCameraMode = bcm_winch;
	}
	else
	{
	    boat->boatCameraMode = bcm_steer;
	}
	v4 zAxis = {0.0f, 1.0f, 0.0f, 0.0f};		
	sail_type* main = &boat->sailInfo.mainSail;

#define MAST 0
	
	if (boat->boatCameraMode == bcm_steer)
	{
	    i32 boatTurnDeg = 1; // * deltaTime ??


	    if (controller->moveLeft.endedDown)
	    {
		boat->currRot = RotateOBJ(boat->objInfo,
					  deltaTime,
					  boat->startRot,
					  &boat->qTargetRot,
					  boat->lerpTimeSpeed,
					  zAxis,
					  (r32)boatTurnDeg);

		CalculateCameraLocation(camera, boat);
		CalculateMastLocation(main, boat);		
#if MAST		


		main->qSailRot = RotateOBJ(boat->mast,
					   deltaTime,
					   main->startRot,
					   &main->qTargetRot,
					   boat->lerpTimeSpeed,
					   zAxis,
					   (r32)boatTurnDeg);
#endif

	    }

	    if (controller->moveRight.endedDown)
	    {

		boat->currRot = RotateOBJ(boat->objInfo,
					  deltaTime,
					  boat->startRot,
					  &boat->qTargetRot,
					  boat->lerpTimeSpeed,
					  zAxis,
					  (r32)-boatTurnDeg);		

		CalculateCameraLocation(camera, boat);
		CalculateMastLocation(main, boat);		
#if MAST


		main->qSailRot = RotateOBJ(boat->mast,
					   deltaTime,
					   main->startRot,
					   &main->qTargetRot,
					   boat->lerpTimeSpeed,
					   zAxis,
					   (r32)-boatTurnDeg);
#endif
	    }
	}
	else if (boat->boatCameraMode == bcm_winch)
	{
	    //change the orientation of the sails

	    r32 sailDeg = 1;
	    if (controller->moveLeft.endedDown)
	    {

		main->qSailRot = RotateOBJ(boat->mast,
					   deltaTime,
					   main->startRot,
					   &main->qTargetRot,
					   boat->lerpTimeSpeed,
					   zAxis,
					   (r32)sailDeg,
					   boat->objInfo->modelMatrix);
						
	    }
	    if (controller->moveRight.endedDown)
	    {

		main->qSailRot = RotateOBJ(boat->mast,
					   deltaTime,
					   main->startRot,
					   &main->qTargetRot,
					   boat->lerpTimeSpeed,
					   zAxis,
					   (r32)-sailDeg,
					   boat->objInfo->modelMatrix);		
	    }
	}
	ProcessSailInputs(controller, boat);

    }
    boat->forward = GetForwardFromQuat(boat->currRot, &boat->pitch, &boat->yaw);    

    gameFrameworkCode->GameUpdateCamera(camera);


}
