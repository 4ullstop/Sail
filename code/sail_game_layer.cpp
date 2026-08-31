#include "sail_game_layer.h"


#include "D:/ExternalCustomAPIs/OBJLoader/code/mtl_parser.cpp"

//Introducing waves

/*
  Boat information:
  Overall length: 8.74m;
  Beam: 3.12m;
 */



//waves v2


//A lot of this information pertaining to the boat dimensions, wave angles etc... may need to be stored
//at some point in the future
//x: x, y: z, z: t

internal v2
RotateV2(v2 v, r32 angleR)
{
    r32 cosA = cosf(angleR);
    r32 sinA = cosf(angleR);
    v2 result = {};
    result.x = v.x * cosA - v.y * sinA;
    result.y = v.x * sinA + v.y * cosA;
    return(result);
}

internal r32 
WaveHeight(v2 points, r32 time)
{
    r32 y = 0.0f;
    r32 amplitude = 0.2f;
    
    r32 waveLength = 10.0f;
    r32 kMag = (2.0f * FM_PI) / waveLength;
    v2 waveDir = {0.7071f, 0.7071f};
    
    v2 k = {waveDir.x * kMag, waveDir.y * kMag}; //If you wanted to get uber technical, you can calculate this value
    //by choosing a wave length in meters and then the angle it is travelling to the x axis
    r32 kMagnitude = sqrtf((k.x * k.x) + (k.y * k.y));
//our angular frequency w 
    r32 w = (r32)sqrtf(kMagnitude * 9.81f);

    r32 phase = (k.x * points.x) + (k.y * points.y) - (w * time);
    y = amplitude * sinf(phase);
    return(y);
}

struct wave_computation
{
    v4 n;
    r32 yPos;
};

internal wave_computation
ComputeWave(boat_entity* boat, r32 boatYaw)
{
    //Get sample points of the boat
    wave_computation result = {};
    r32 boatL = 8.74f;
    r32 boatW = 3.12f;

    r32 time = boat->waveInfo.t;
    
    v2 bowL = {0.0f, boatL / 2.0f};
    v2 sternL = {0.0f, -(boatL / 2.0f)};
    v2 portL = {-(boatW / 2.0f), 0.0f};
    v2 starboardL = {boatW / 2.0f, 0.0f};
    
    bowL = RotateV2(bowL, boatYaw);
    sternL = RotateV2(sternL, boatYaw);
    portL = RotateV2(portL, boatYaw);
    starboardL = RotateV2(starboardL, boatYaw);
    
    v2 worldPos = {boat->objInfo->modelTransform.location.x, boat->objInfo->modelTransform.location.z};

    v2 bowW = worldPos + bowL;
    v2 sternW = worldPos + sternL;
    v2 portW = worldPos + portL;
    v2 starboardW = worldPos + starboardL;

    r32 yBow = WaveHeight(bowW, time);
    r32 yStern = WaveHeight(sternW, time);
    r32 yPort = WaveHeight(portW, time);
    r32 yStarboard = WaveHeight(starboardW, time);



    r32 yBmyS = yBow - yStern;
    r32 ySmyP = yStarboard - yPort;
    
    r32 boatY = (yBow + yStern + yPort + yStarboard) / 4.0f;
    result.yPos = boatY;
    r32 pitch = yBmyS / boatL;
    r32 roll = ySmyP / boatW;

    v4 longitudinal = {0.0f, yBmyS, boatL, 0.0f};
    v4 transverse = {boatW, ySmyP, 0.0f, 0.0f};

    v4 n = NormalizeV4(CrossV3(transverse, longitudinal));
    result.n = n;
    return(result);
}

//Sailing mechanic

internal v4
GetForwardFromQuat(v4 inQuat, r32* pitch, r32* yaw)
{
    v4 quat = QuaternionNormalize(inQuat);
//    quat = QuaternionConjugate(quat);

    r32 forwardX = -2.0f * (quat.x * quat.z + quat.y * quat.w);
    r32 forwardZ = -(1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y));

    *yaw = atan2f(forwardX, forwardZ);
    v4 result = GetForwardVector(*pitch, *yaw);

//    result.x = -result.x;
    return(result);
}

internal bool32
IsAngleRightSide(r32 angle)
{
    if (angle < 0.0f)
    {
	if ((angle >= -90.0f) && (angle < 0.0f))
	{
	    return(true);
	}
	else
	{
	    return(false);
	}
    }
    else
    {
	if ((angle >= 0.0f) && (angle < 90.0f))
	{
	    return(true);
	}
	else
	{
	    return(false);
	}
    }    
}

internal r32
CalculateRegularDropOff(r32 x, r32 y, r32 radius)
{
    r32 distance = (r32)fabs(y - x);
    r32 z = 1.0f - (distance / radius);
    r32 result = (r32)Max(0.0f, Min(z, 1.0f));
    return(result);
}

internal r32
CalculateSmoothDropOff(r32 x, r32 y, r32 sigma)
{
    r32 delta = y - x;
    r32 result = (r32)expf(-(delta * delta) / (2.0f * sigma * sigma));
    return(result);
}

internal void
UpdateBoatVectors(boat_entity* boat)
{
    boat->forward = GetForwardFromQuat(boat->currRot, &boat->pitch, &boat->yaw);
    sail_type* main = &boat->sailInfo.mainSail;
    main->sailForward = GetForwardFromQuat(main->qSailRot, &main->pitch, &main->yaw);
    main->sailForward = NormalizeV3(main->sailForward);
    main->sailForward = NegateVector(main->sailForward);
    //Now that we have the forwards of both of the objects we can compare against the boat and the wind direction

    //boat vs wind dir
    //sail vs wind dir

    //Get the signed axis between the sail and the wind to determine if the sail is in the wrong
    //direction to the wind

    r32 s = 0.0f;
    v2 u = {boat->forward.x, boat->forward.z};
    v2 v = {-boat->sailInfo.windDirection.x, -boat->sailInfo.windDirection.z};
    r32 angle = (r32)atan2((u.x * v.y) - (u.y * v.x), (u.x * v.x) + (u.y * v.y));
    boat->windAngle = (r32)RAD2DEG(angle);

    v = {main->sailForward.x, main->sailForward.z};
    angle = (r32)atan2((u.x * v.y) - (u.y * v.x), (u.x * v.x) + (u.y * v.y));

    boat->sailAngle = (r32)RAD2DEG(angle);

    r32 diff = (r32)(fabs(fabs(boat->windAngle) - fabs(boat->sailAngle)));
    r32 half = (r32)fabs(boat->windAngle / 2.0f);

    //Okay lets get crazy and use it twice
    if ((boat->windAngle > - 15.0f) && (boat->windAngle < 15.0f))
    {
	if (boat->sailAngle < 0.0f)
	{
	    half = 180.0f;
	}
    }
    
    
    r32 closerToZero = CalculateRegularDropOff(0.0f, half, 5.0f);
    r32 sigma = Lerp(4.0f, 7.0f, closerToZero);
    
    s = CalculateSmoothDropOff(half, diff, 8);

    
    u = v;
    v = {boat->forward.x, boat->forward.z};

    angle = (r32)atan2((u.x * v.y) - (u.y * v.x), (u.x * v.x) + (u.y * v.y));    

    bool32 sailIsRight = IsAngleRightSide(boat->sailAngle);
    bool32 boatIsRight = IsAngleRightSide(boat->windAngle);

    i32 sign = 1;
    
    boat->boatToSail = (r32)RAD2DEG(angle);
    
    if ((boat->windAngle > 0.0f) && (boat->sailAngle > 0.0f))
    {
	s = 0.1f;
	sign = -1;
    }

    r32 p = Lerp(boat->bottomSpeed, boat->topSpeed, s);

    boat->movementSpeed = p * sign;
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

    cameraResult.yaw = -0.0f;
    cameraResult.pitch = 0.0f;
    cameraResult.front = {0.0f, 0.0f, -1.0f, 0.0f};
    cameraResult.position = {-8.2f, 0.04f, 0.77f, 0.0f};
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

    //this seems really large and really weird, how can we make our objs smaller? less faces
    size_t objectArenaAllocSize = Megabytes(50);

    platformInfo->frameworkArenas.spawnedObjectArena =
	(memory_arena*)memoryPoolCode->PushStruct(platformInfo->frameworkArenas.setupArena, sizeof(memory_arena));
    
    memoryPoolCode->InitArena(platformInfo->frameworkArenas.spawnedObjectArena,
			      objectArenaAllocSize,
			      pgMem,
			      e_arena_type::permanent);



    char* icoPath = "../data/obj/debug_ico.obj";
    char* boatPath = "../data/obj/boat_V3.obj";
    char* mastPath = "../data/obj/boat_V2_mast.obj";
    char* refCubePath = "../data/obj/move_ref.obj";
    char* windSockPath = "../data/obj/wind_sock.obj";
    char* axesPath = "../data/obj/axes.obj";
    char* arrowPath = "../data/obj/forward_arrow.obj";
    char* texTestPath = "../data/obj/texture_testing.obj";
    char* oceanPath = "../data/obj/ocean_V1.obj";
    char* windDirModelBottom = "../data/obj/boat_wind_dir_bottom.obj";
    char* windDirModelTop = "../data/obj/boat_wind_dir_top.obj";
    char* paths[256] = {boatPath, mastPath, refCubePath, windSockPath, axesPath, arrowPath, texTestPath, oceanPath, windDirModelBottom, windDirModelTop};

    
    initData->gameObjs = gameFrameworkCode->GameLoadOBJFiles(platformInfo->parseObjCode,
							     &platformInfo->frameworkArenas,
							     pgMem, memoryPoolCode, paths, 10);

    initData->isFreeCam = false;
    char* testPath = "../data/textures/cat_tester.bmp";
    char* windModelTexture = "../data/textures/boat_wind_dir_uvs.bmp";
    char* texPaths[256] = {testPath, windModelTexture};
    initData->gameTextures = gameFrameworkCode->GameLoadTextures(texPaths,
								 platformInfo->frameworkArenas.setupArena,
								 2,
								 DEBUGPlatformReadEntireFile,
								 memoryPoolCode);
    
#if 0    
    char* filename = "../data/obj/axes.mtl";
    ParseMTLData(filename,
		 platformInfo->frameworkArenas.perFrameArena,
		 platformInfo->frameworkArenas.setupArena,
		 pgMem,
		 memoryPoolCode);
#endif
    v4 spawnObjLoc = v4{0.0f, 0.0f, 10.0f, 1.0f};
    v4 oneScale = {1.0f, 1.0f, 1.0f, 1.0f};
#if 0
    gameFrameworkCode->GameSpawnNewOBJ(spawnable_obj_type::sot_ico,
				       spawnObjLoc,
				       &initData->gameObjs,
				       memoryPoolCode);
#else

    v4 axesRot = {1.0f, 0.0f, 0.0f, 0.0f};

    transform oceanTransform = {};
    oceanTransform.location = {0.0f, -1.0f, 0.0f, 0.f};
    oceanTransform.rotation = QuaternionIdentity();
    oceanTransform.scale = oneScale;

    gameFrameworkCode->GameSpawnNewOBJ(sot_ocean,
				       oceanTransform,
				       &initData->gameObjs,
				       memoryPoolCode,
				       false,
				       Identity());
    
    transform texTestTransform = {};
    texTestTransform.location = {4.0f, 0.0f, 0.0f};
    texTestTransform.rotation = QuaternionIdentity();
    texTestTransform.scale = oneScale;
    spawned_obj_info* testTextureOBJ = gameFrameworkCode->GameSpawnNewOBJ(spawnable_obj_type::sot_texTest,
							     texTestTransform,
							     &initData->gameObjs,
							     memoryPoolCode,
							     false,
							     Identity());
    testTextureOBJ->textureInfo = tl_testing;
    
    transform axesTransform = {};
    axesTransform.location = {0.0f, 0.0f, 0.0f, 0.0f};
    axesTransform.rotation = QuaternionIdentity();
    axesTransform.scale = oneScale;
    gameFrameworkCode->GameSpawnNewOBJ(spawnable_obj_type::sot_axes,
				       axesTransform,
				       &initData->gameObjs,
				       memoryPoolCode,
				       false,
				       Identity());

    transform forwardArrowTransform = {};
    forwardArrowTransform.location = {0.0f, 0.0f, 0.0f, 0.0f};
    forwardArrowTransform.rotation = CreateQuaternionRotationFromVector(axesRot);
    forwardArrowTransform.scale = oneScale;

    gameFrameworkCode->GameSpawnNewOBJ(spawnable_obj_type::sot_forward_arrow,
				       forwardArrowTransform,
				       &initData->gameObjs,
				       memoryPoolCode,
				       false,
				       Identity());
    
    
    transform refCubeTransform = {};
    refCubeTransform.location = {6.0f, 0.0f, -10.0f, 0.0f};
    refCubeTransform.rotation = QuaternionIdentity();
    refCubeTransform.scale = oneScale;

    gameFrameworkCode->GameSpawnNewOBJ(spawnable_obj_type::sot_ref,
				       refCubeTransform,
				       &initData->gameObjs,
				       memoryPoolCode,
				       false,
				       Identity());
    
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

    //WAVES

    //MAIN SAIL

    transform windModelBottomTransform = {};
    windModelBottomTransform.location = {-0.4f, 0.3f, 1.8f, 0.0f};
    windModelBottomTransform.rotation = QuaternionIdentity();
    windModelBottomTransform.scale = oneScale;
    initData->boat.windModelBottom = 
	gameFrameworkCode->GameSpawnNewOBJ(sot_wind_model_bottom,
					   windModelBottomTransform,
					   &initData->gameObjs,
					   memoryPoolCode,
					   true,
					   initData->boat.objInfo->modelMatrix);
    initData->boat.windModelBottom->textureInfo = tl_wind_model;
    
    transform windModelTopTransform = {};
    windModelTopTransform.location = {0.0f, 0.0f, 0.0f, 0.0f};
    windModelTopTransform.rotation = QuaternionIdentity();
    windModelTopTransform.scale = oneScale;
    initData->boat.windModelTop =
	gameFrameworkCode->GameSpawnNewOBJ(sot_wind_model_top,
					   windModelTopTransform,
					   &initData->gameObjs,
					   memoryPoolCode,
					   true,
					   initData->boat.windModelBottom->modelMatrix);

    initData->boat.windModelTop->textureInfo = tl_wind_model;
//    v4 mastLocation = {0.65f, -0.7f, 0.0f, 1.0f};
    v4 mastLocation = {0.0f, 0.0f, -1.0f, 1.0f};
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

    
    initData->boat.windSock = {};
    
    transform windSockTransform = {};
    windSockTransform.location = {0.0f, 0.0f, 5.0f, 0.0f};
    windSockTransform.rotation = QuaternionIdentity();
    windSockTransform.scale = oneScale;

    initData->boat.windSock.model =
	gameFrameworkCode->GameSpawnNewOBJ(spawnable_obj_type::sot_windsock,
					   windSockTransform,
					   &initData->gameObjs,
					   memoryPoolCode,
					   true,
					   initData->boat.objInfo->modelMatrix);
    
    //BOAT
#if 0    
    v4 camOffset = {-4.2f, 0.04f, 0.77f, 0.0f};
#else
    v4 camOffset = {0.0f, 1.0f, 3.0f, 0.0f};    
#endif    
    
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
    initData->boat.leftCamOffset = {camOffset.x - 1.0f, camOffset.y, camOffset.z, camOffset.w};
    initData->boat.rightCamOffset = {camOffset.x + 1.0f, camOffset.y, camOffset.z, camOffset.w};


    initData->boat.topSpeed = 5.f;
    initData->boat.movementSpeed =     
	initData->boat.bottomSpeed = 1.0f;

    initData->boat.sailInfo.windDirection = v4{1.0f, 0.0f, 0.0f, 0.0f};
    
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
RotateOBJ(spawned_obj_info* obj, r32 roll, r32 pitch, r32 yaw, v4 qCurrRot, r32 deltaTime)
{
    v3 result = {};
    v4 targetQuat = QuaternionFromEuler((r32)DEG2RAD(pitch), (r32)DEG2RAD(yaw), (r32)DEG2RAD(roll));
    targetQuat = QuaternionNormalize(targetQuat);

    r32 rate = 1.0f;
    r32 slerpFactor = 1.0f - (r32)expf(-deltaTime * rate);
    if (slerpFactor > 1.0f) slerpFactor = 1.0f;

    v4 t4 = {slerpFactor, slerpFactor, slerpFactor, slerpFactor};

    
    v4 outRotation = QuaternionSlerpV(qCurrRot, targetQuat, t4);
    obj->modelTransform.rotation = outRotation;
    obj->modelMatrix = CreateModelMatrix(obj->modelTransform.scale,
					 obj->modelTransform.rotation,
					 obj->modelTransform.location);
    return(outRotation);
}

internal v4
RotateOBJ(spawned_obj_info* obj, v3 targetRotations, v4 qCurrRot, r32 deltaTime)
{
    return(RotateOBJ(obj, targetRotations.roll, targetRotations.roll, targetRotations.pitch, qCurrRot, deltaTime));
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
TranslateOBJ(spawned_obj_info* objInfo, v4 start, v4 target, r32 lerpSpeed, r32 deltaTime, m4 parentTransform)
{
    r32 t = 1 - lerpSpeed * deltaTime;
    v4 t4 = {t, t, t, t};
    v4 outPos = VectorLerp(start, target, t4);
    
    if(objInfo)
    {
	objInfo->modelTransform.location = outPos;
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

    return(outPos);
}

internal v4
TranslateOBJ(spawned_obj_info* objInfo, v4 start, v4 target, r32 lerpSpeed, r32 deltaTime)
{
    return(TranslateOBJ(objInfo, start, target, lerpSpeed, deltaTime, Identity()));
}

internal v4
RotateOBJ(spawned_obj_info* objInfo, r32 deltaTime, v4 startRot, v4* targetRot, r32 lerpSpeed, v4 axis, r32 degrees)
{
    return(RotateOBJ(objInfo, deltaTime, startRot, targetRot, lerpSpeed, axis, degrees, Identity()));
}

internal v4
RotateOBJLocal(spawned_obj_info* objInfo, r32 deltaTime, v4 startRot, v4* targetRot, r32 lerpSpeed, v4 axis, r32 degrees)
{
    *targetRot = QuaternionNormalize(*targetRot);
    *targetRot = QuaternionMultiply(*targetRot,
				    QuaternionRotationAxis(axis, (r32)DEG2RAD(degrees)));
    
    r32 t = 1 - lerpSpeed * deltaTime;
    v4 t4 = {t, t, t, t};
    v4 outRotation = QuaternionSlerpV(startRot, *targetRot, t4);

    if (objInfo)
    {
	objInfo->localTransform.rotation = outRotation;
	objInfo->localMatrix = CreateModelMatrix(objInfo->localTransform.scale,
						 objInfo->localTransform.rotation,
						 objInfo->localTransform.location);
    }
    return(outRotation);
}


internal void
ApplyBoatWaveRotations(boat_entity* boat, r32 deltaTime)
{

    r32 currentYawAngle = (r32)DEG2RAD(boat->targetBoatRotations.yaw);
    
    v4 qYaw =
    {
	0.0f,
	sinf(currentYawAngle * 0.5f),
	0.0f,
	cosf(currentYawAngle * 0.5f)
    };
    
    r32 timeStep = 0.3f;
    boat->waveInfo.t += timeStep * deltaTime;
    wave_computation wave = ComputeWave(boat, currentYawAngle);
    v4 qWave = wave.n;
    v4 localUp = {0.0f, 1.0f, 0.0f, 0.0f};
    v4 v = CrossV3(localUp, qWave);


    
    v4 aLenSq = VecLenSq(localUp);
    v4 bLenSq = VecLenSq(qWave);

    v3 aLenSq3 = {aLenSq.x, aLenSq.y, aLenSq.z};
    v3 bLenSq3 = {bLenSq.x, bLenSq.y, bLenSq.z};
    v3 a = {localUp.x, localUp.y, localUp.z};
    v3 b = {qWave.x, qWave.y, qWave.z};
    
    r32 w = (r32)(sqrtf(Dot(aLenSq3, bLenSq3))) + (Dot(a, b));
    v4 quat = {v.x, v.y, v.z, w};
    v4 qTilt = QuaternionNormalize(quat);
    v4 targetRot = QuaternionMultiply(qTilt, qYaw);
    
    v4 addedPos = {0.0f, wave.yPos, 0.0f, 0.0f};

    r32 baseWaterLevel = 0.0f;
    boat->objInfo->modelTransform.location.y = baseWaterLevel + wave.yPos;

    


    v4 t4 = {timeStep, timeStep, timeStep, timeStep};
    v4 outRotation = QuaternionSlerpV(boat->currRot, targetRot, t4);
    boat->currRot = outRotation;
    boat->objInfo->modelTransform.rotation = outRotation;
    boat->objInfo->modelMatrix = CreateModelMatrix(boat->objInfo->modelTransform.scale,
						   boat->objInfo->modelTransform.rotation,
						   boat->objInfo->modelTransform.location);


}

internal void
UpdateMastModelMatrix(sail_type* main, boat_entity* boat)
{
    boat->mast->modelMatrix = boat->mast->localMatrix * boat->objInfo->modelMatrix;
    boat->windSock.model->modelMatrix = boat->windSock.model->localMatrix * boat->objInfo->modelMatrix;
}

internal void
UpdateChildModelMatrix(spawned_obj_info* child, spawned_obj_info* parent)
{
    child->modelMatrix = child->localMatrix * parent->modelMatrix;
}




extern "C" SAIL_UPDATE(SailUpdate)
{
    //Update our input
    //Update our camera

    game_controller_input* controller = GetController(input, 0);    

    boat_entity* boat = &initData->boat; 

    if (controller)
    {
	if (controller->one.started)
	{
	    initData->isFreeCam = !initData->isFreeCam;
	    controller->one.started = false;
	}
	if (initData->isFreeCam)
	{
	    r32 camVelocity = camera->movementSpeed * deltaTime;
	    if (controller->moveForward.endedDown)
	    {
		//w
		camera->position = camera->position + (camera->front * camVelocity);
	    }

	    if (controller->moveLeft.endedDown)
	    {
		//a
		camera->position = camera->position - (camera->right * camVelocity);
	    }

	    if (controller->moveBackward.endedDown)
	    {
		//s
		camera->position = camera->position - (camera->front * camVelocity);	    
	    }

	    if (controller->moveRight.endedDown)
	    {
		//d
		camera->position = camera->position + (camera->right * camVelocity);
	    }



	}
	else
	{

	    //eventually, it would be nice if the movement was also dependent
	    //on the velocity of the boat, meaning we turn more or less depending on how fast the boat
	    //is moving or if the boat is moving at all
	    //Q

	    sail_type* main = &boat->sailInfo.mainSail;

	    r32 velocity = boat->movementSpeed * deltaTime;

	    wind_sock* windSock = &boat->windSock;

	
	    boat->objInfo->modelTransform.location = boat->objInfo->modelTransform.location + (boat->forward * velocity);
	    boat->objInfo->modelMatrix = CreateModelMatrix(boat->objInfo->modelTransform.scale,
							   boat->objInfo->modelTransform.rotation,
							   boat->objInfo->modelTransform.location);


	    CalculateCameraLocation(camera, boat);
	    UpdateMastModelMatrix(main, boat);


	    
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



	    
	    r32 turnSpeed = 4.0f;
	    r32 turnSlerp = 10.0f;
	    r32 currYaw = boat->targetBoatRotations.yaw;

	    if (boat->boatCameraMode == bcm_steer)
	    {
		i32 boatTurnDeg = 1; // * deltaTime ??


		if (controller->moveLeft.endedDown)
		{

		    boat->currTargetYaw = boat->targetBoatRotations.y + turnSpeed;



//		    boat->targetBoatRotations.yaw += turnSpeed * deltaTime;

		    
#if 0		    
		    boat->currRot = RotateOBJ(boat->objInfo,
					      boat->targetBoatRotations,
					      boat->currRot,
					      deltaTime);



		    CalculateCameraLocation(camera, boat);
		    UpdateMastModelMatrix(main, boat);
#endif				    
		}

		
		if (controller->moveRight.endedDown)
		{


		    boat->currTargetYaw = boat->targetBoatRotations.y - turnSpeed;


//		    boat->targetBoatRotations.yaw -= turnSpeed * deltaTime;
#if 0		    
		    boat->currRot = RotateOBJ(boat->objInfo,
					      boat->targetBoatRotations,
					      boat->currRot,
					      deltaTime);		    

		    CalculateCameraLocation(camera, boat);
		    UpdateMastModelMatrix(main, boat);
#endif		    
		}
	    }
	    else if (boat->boatCameraMode == bcm_winch)
	    {
		//change the orientation of the sails

		/*
		  then flesh out some of the kinks with the rotation and wind direction stuff
		  then make a input buffer to make it so you have to 'wind' the sails in each direction
		  using 'asdwa' or 'dsawd' depending on which way you want to rotate the sail
		*/
		r32 sailDeg = 1;
		if (boat->staticCamLocation == static_cam_location::scl_left)
		{
		    if (controller->moveLeft.endedDown)
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
		else if (boat->staticCamLocation == static_cam_location::scl_right)
		{
		    if (controller->moveRight.endedDown)
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
		}
	    


	    }


	    
	    


	    boat->targetBoatRotations.yaw = Lerp(currYaw, boat->currTargetYaw, 1.0f - expf(-deltaTime * turnSlerp));

	    boat->currRot = RotateOBJ(boat->objInfo,
				      boat->targetBoatRotations,
				      boat->currRot,
				      deltaTime);


	    
	    ApplyBoatWaveRotations(boat, deltaTime);	
	    ProcessSailInputs(controller, boat);
	    UpdateBoatVectors(boat);


	    CalculateCameraLocation(camera, boat);
	    UpdateMastModelMatrix(main, boat);			    
	    //Updating the wind sock so I can tell what direction the wind is going

	    v4 clearWind = NormalizeV4(boat->sailInfo.windDirection);

	    v4 windRotation = CreateQuaternionRotationFromVector(clearWind);
	    windRotation = QuaternionNormalize(windRotation);
	    v4 offset = {0.7f, -0.4f, 5.2f, 0.0f};
	    m4 windMat = MatrixRotationQuaternion(windRotation);
	

	    v4 normalBoatRot = QuaternionNormalize(boat->objInfo->modelTransform.rotation);
	    m4 boatRotation = MatrixRotationQuaternion(normalBoatRot);

	    v4 rotatedOffset = Vector3Transform(offset, boatRotation);

	    v4 boatPos = boat->objInfo->modelTransform.location;
	
	    windMat.e[3][0] = boatPos.x + rotatedOffset.x;
	    windMat.e[3][1] = boatPos.y + rotatedOffset.y;
	    windMat.e[3][2] = boatPos.z + rotatedOffset.z;
	    windMat.e[3][3] = 1.0f;

	    windSock->model->modelMatrix = windMat;

	    UpdateChildModelMatrix(boat->windModelBottom, boat->objInfo);
	    UpdateChildModelMatrix(boat->windModelTop, boat->windModelBottom);
	}
    }

    gameFrameworkCode->GameUpdateCamera(camera);

}
