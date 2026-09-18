#include "sail_game_layer.h"
#include <stdlib.h>

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
internal v3
FromV4ToV3Rotations(v4 v)
{
    v3 result =
    {
	v.pitch,
	v.yaw,
	v.roll
    };
    return(result);
}

r32 Clamp(r32 value, r32 min, r32 max)
{
    if (value < min) return min;
    if (value > max) return max;
    return (value);
}

internal r32
RandomNumberBetween(r32 min, r32 max)
{
    i32 iMin = (i32)min;
    i32 iMax = (i32)max;
    i32 iOut = (rand() % (iMax - iMin + 1)) + iMin;
    r32 result = (r32)iOut;
    return(result);
}

internal r32
RandomFloatInRange(r32 min, r32 max)
{
    r32 result = ((r32)rand() / (r32)RAND_MAX) * (max - min) + min;
    return(result);
}

internal v4
GetForwardFromQuat(v4 inQuat, r32* pitch, r32* yaw)
{
    v4 quat = QuaternionNormalize(inQuat);

    r32 forwardX = 2.0f * (quat.x * quat.z + quat.y * quat.w);
    r32 forwardY = 2.0f * (quat.y * quat.z - quat.x * quat.w);
    r32 forwardZ = 1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y);

    if (pitch)
    {
	*pitch = asinf(Clamp(forwardY, -1.0f, 1.0f));
    }
    if (yaw)
    {
	*yaw = atan2f(forwardX, forwardZ);
    }

    v4 result = GetForwardVector(*pitch, *yaw);
    return(result);
}

internal v4
GetForwardFromQuat(v4 inQuat, r32 pitch, r32 yaw)
{
    v4 quat = QuaternionNormalize(inQuat);

    r32 forwardX = 2.0f * (quat.x * quat.z + quat.y * quat.w);
    r32 forwardY = 2.0f * (quat.y * quat.z - quat.x * quat.w);
    r32 forwardZ = 1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y);

    if (pitch)
    {
	pitch = asinf(Clamp(forwardY, -1.0f, 1.0f));
    }
    if (yaw)
    {
	yaw = atan2f(forwardX, forwardZ);
    }

    v4 result = GetForwardVector(pitch, yaw);
    return(result);    
}

//supply in degrees
internal v3
AddTargetYawRotation(r32 rotationAdditive, v3 currentRot)
{
    v3 result = {};
    result.yaw = currentRot.yaw + rotationAdditive;
    if (result.yaw > 360.0f)
    {
	r32 diff = result.yaw - 360.0f;
	result.yaw = (r32)fabs(diff);
    }
    return(result);
}

internal void
LerpUpdateRotationsYaw(rotational_update* inRots, r32 deltaTime)
{
    if (inRots->inRotation)
    {
	inRots->currentLerp += inRots->lerpSpeed * deltaTime;
	inRots->eCurrentRotations.yaw = Lerp(inRots->eStartRotations.yaw, inRots->eTargetRotations.yaw, inRots->currentLerp);

	v4 targetQuat = QuaternionFromEuler((r32)DEG2RAD(inRots->eCurrentRotations.pitch),
					    (r32)DEG2RAD(inRots->eCurrentRotations.yaw),
					    (r32)DEG2RAD(inRots->eCurrentRotations.roll));
	inRots->qCurrent = QuaternionNormalize(targetQuat);

	inRots->vCurrent = GetForwardFromQuat(targetQuat,
					      (r32)DEG2RAD(inRots->eCurrentRotations.pitch),
					      (r32)DEG2RAD(inRots->eCurrentRotations.yaw));
	if (inRots->currentLerp >= 1.0f)
	{
	    inRots->inRotation = false;
	}
    }
    else if ((inRots->eTargetRotations.yaw != inRots->eCurrentRotations.yaw) && (!inRots->inRotation))
    {
	inRots->currentLerp = 0.0f;
	inRots->inRotation = true;
	inRots->eStartRotations = inRots->eCurrentRotations;
    }
}

//currentWindDirection is input as a forward
internal void
UpdateWindDirection(wind_rotation_update* currentWindInfo, v4* currentWindDirection, r32 deltaTime)
{
    if (currentWindInfo->windInRotation)
    {
	currentWindInfo->currentLerp += 0.3f * deltaTime;
	currentWindInfo->eCurrentRotations.yaw = Lerp(currentWindInfo->eStartRotations.yaw, currentWindInfo->eTargetRotations.yaw, currentWindInfo->currentLerp);

	v4 targetQuat = QuaternionFromEuler((r32)DEG2RAD(currentWindInfo->eCurrentRotations.pitch),
					    (r32)DEG2RAD(currentWindInfo->eCurrentRotations.yaw),
					    (r32)DEG2RAD(currentWindInfo->eCurrentRotations.roll));
	targetQuat = QuaternionNormalize(targetQuat);

	*currentWindDirection = GetForwardFromQuat(targetQuat,
						   (r32)DEG2RAD(currentWindInfo->eCurrentRotations.pitch),
						   (r32)DEG2RAD(currentWindInfo->eCurrentRotations.yaw));

	
	if (currentWindInfo->currentLerp >= 1.0f)
	{
	    currentWindInfo->windInRotation = false;
	}
    }
    else if ((currentWindInfo->eTargetRotations.yaw != currentWindInfo->eCurrentRotations.yaw) && (!currentWindInfo->windInRotation))
    {
	currentWindInfo->currentLerp = 0.0f;
	currentWindInfo->windInRotation = true;
	currentWindInfo->eStartRotations = currentWindInfo->eCurrentRotations;
    }
}

inline r32
MapValue(v2 oRange, v2 nRange, r32 value)
{
    r32 result = (nRange.x + value) * ((nRange.y - nRange.x) / (oRange.y - oRange.x));
    return(result);
}
    
#define PADSENS 2.0f
#define SAILMOVEMENTSENS 0.003f
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

internal void
AddNoiseToBoat(void)
{
    
}

internal void
AddNoiseToWaves(wave_properties* properties)
{
    r32 rand = RandomFloatInRange(2.0f, 5.0f);
    properties->waveRotations.eCurrentRotations = AddTargetYawRotation(rand, properties->waveRotations.eStartRotations);
}

internal r32 
WaveHeight(v2 points, r32 time, wave_properties* properties)
{
    r32 y = 0.0f;

    r32 kMag = (2.0f * FM_PI) / properties->waveLength;
    v4 waveForward = GetForwardVector((r32)DEG2RAD(properties->eWaveDirection.pitch),
						   (r32)DEG2RAD(properties->eWaveDirection.yaw));

    v2 waveDir = {waveForward.x, waveForward.z};

    v2 k = {waveDir.x * kMag, waveDir.y * kMag}; //If you wanted to get uber technical, you can calculate this value
    //by choosing a wave length in meters and then the angle it is travelling to the x axis
    r32 kMagnitude = sqrtf((k.x * k.x) + (k.y * k.y));
//our angular frequency w 
    r32 w = (r32)sqrtf(kMagnitude * 9.81f);

    r32 phase = (k.x * points.x) + (k.y * points.y) - (w * time);
    y = properties->amplitude * sinf(phase);
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

    r32 yBow = WaveHeight(bowW, time, &boat->waveInfo.properties);
    r32 yStern = WaveHeight(sternW, time, &boat->waveInfo.properties);
    r32 yPort = WaveHeight(portW, time, &boat->waveInfo.properties);
    r32 yStarboard = WaveHeight(starboardW, time, &boat->waveInfo.properties);



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
    boat->forward = NegateVector(NormalizeV4(boat->forward));
    sail_type* main = &boat->sailInfo.mainSail;

    main->sailForward = GetForwardVector(main->targetSailRotations.pitch, main->targetSailRotations.yaw);
    main->sailForward = NormalizeV3(main->sailForward);

    //Now that we have the forwards of both of the objects we can compare against the boat and the wind direction

    //boat vs wind dir
    //sail vs wind dir

    //Get the signed axis between the sail and the wind to determine if the sail is in the wrong
    //direction to the wind

    r32 s = 0.0f;
    v2 u = {boat->forward.x, boat->forward.z};
    //windDirection will be stored as a forward vector, if you need to change the direction you must convert it
    //from the desired rotation when assigning it!!!
    v2 v = {boat->sailInfo.windDirection.x, boat->sailInfo.windDirection.z};
    r32 angle = UnsignedAngleBetweenTwoVectorsDegrees(u, v);
    boat->windAngle = (angle);

    r32 relAngle = main->targetSailRotations.yaw;
    while (relAngle > 180.0f) relAngle -= 360.0f;
    while (relAngle < -180.0f) relAngle += 360.0f;


    boat->sailAngle = (r32)RAD2DEG(relAngle);

    v2 boatPortRange = {0.0f, -180.0f};
    v2 boatStarboardRange = {0.0f, 180.0f};

    v2 sailRange = {0.0f, 90.0f};
    r32 targetRange = 0.0f;

    if (boat->windAngle < 0.0f)
    {
	targetRange = MapValue(boatPortRange, sailRange, boat->windAngle);

	if (boat->windAngle > -45.0f)
	{
	    boat->movementDirection = bmd_waves;
	}
	else
	{
	    boat->movementDirection = bmd_forward;
	}
    }
    else
    {
	targetRange = -MapValue(boatStarboardRange, sailRange, boat->windAngle);
	if (boat->windAngle < 45.0f)
	{
	    boat->movementDirection = bmd_waves;
	}
	else
	{
	    boat->movementDirection = bmd_forward;
	}
    }

    //higher number == move forgivness in sail rotation
    s = CalculateSmoothDropOff(boat->sailAngle, targetRange, 12);

    r32 p = 0.0f;
    if (boat->movementDirection == bmd_waves)
    {
	s = CalculateSmoothDropOff(boat->windAngle, 0.0f, 12);
	p = Lerp(0.0f, 0.2f, s);
    }
    else
    {
	p = Lerp(boat->bottomSpeed, boat->topSpeed, s);	
    }

    boat->movementSpeed = p;
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
    char* windDirCardinal = "../data/obj/boat_wind_dir_cardinal.obj";
    char* marooned = "../data/obj/marooned_V1.obj";
    char* winch = "../data/obj/winch.obj";
    char* speedometerBottom = "../data/obj/speedometer_bottom_v1.obj";
    char* speedometerTop = "../data/obj/speedometer_top_v1.obj";    
    char* paths[512] = {boatPath, mastPath, refCubePath, windSockPath, axesPath, arrowPath, texTestPath, oceanPath, windDirModelBottom, windDirModelTop, marooned, windDirCardinal, winch, speedometerBottom, speedometerTop};

    
    initData->gameObjs = gameFrameworkCode->GameLoadOBJFiles(platformInfo->parseObjCode,
							     &platformInfo->frameworkArenas,
							     pgMem, memoryPoolCode, paths, 15);

    initData->isFreeCam = false;
    char* testPath = "../data/textures/cat_tester.bmp";
    char* windModelTexture = "../data/textures/boat_wind_dir_uvs_v2.bmp";
    char* speedometerTexture = "../data/textures/speedometer_uvs.bmp";
    char* boatTexture = "../data/textures/boat_uv.bmp";
    char* texPaths[256] = {testPath, windModelTexture, speedometerTexture, boatTexture};
    initData->gameTextures = gameFrameworkCode->GameLoadTextures(texPaths,
								 platformInfo->frameworkArenas.setupArena,
								 4,
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
    
    transform maroonedTransform = {};
    maroonedTransform.location = {0.0f, 0.0f, -50.0f};
    maroonedTransform.rotation = QuaternionIdentity();
    maroonedTransform.scale = oneScale;

    gameFrameworkCode->GameSpawnNewOBJ(sot_marooned,
				       maroonedTransform,
				       &initData->gameObjs,
				       memoryPoolCode,
				       false,
				       Identity());
    
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
    
    transform windCardinalTransform = {};
    windCardinalTransform.location = {-0.4f, 0.3f, 1.8f, 0.0f};
    windCardinalTransform.rotation = QuaternionIdentity();
    windCardinalTransform.scale = oneScale;

    initData->boat.windCardinal =
	gameFrameworkCode->GameSpawnNewOBJ(sot_wind_dir_cardinal,
					   windCardinalTransform,
					   &initData->gameObjs,
					   memoryPoolCode,
					   true,
					   initData->boat.objInfo->modelMatrix);

    initData->boat.windCardinal->textureInfo = tl_wind_model;
    

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
    windSockTransform.location = {0.0f, 0.0f, 3.0f, 0.0f};
    windSockTransform.rotation = QuaternionIdentity();
    windSockTransform.scale = oneScale;

    initData->boat.windSock.model =
	gameFrameworkCode->GameSpawnNewOBJ(spawnable_obj_type::sot_windsock,
					   windSockTransform,
					   &initData->gameObjs,
					   memoryPoolCode,
					   true,
					   initData->boat.objInfo->modelMatrix);

    transform winchLTransform = {};
    winchLTransform.location = {-1.0f, 0.4f, 2.3f};
    winchLTransform.rotation = QuaternionIdentity();
    winchLTransform.scale = oneScale;
    initData->boat.winchL.winchModel = 
	gameFrameworkCode->GameSpawnNewOBJ(sot_winch,
					   winchLTransform,
					   &initData->gameObjs,
					   memoryPoolCode,
					   true,
					   initData->boat.objInfo->modelMatrix);
    initData->boat.winchL.targetRot = boatTransform.rotation;    

    transform winchRTransform = {};
    winchRTransform.location = {1.0f, 0.4f, 2.3f};
    winchRTransform.rotation = QuaternionIdentity();
    winchRTransform.scale = oneScale;
    initData->boat.winchR.winchModel = 
	gameFrameworkCode->GameSpawnNewOBJ(sot_winch,
					   winchRTransform,
					   &initData->gameObjs,
					   memoryPoolCode,
					   true,
					   initData->boat.objInfo->modelMatrix);
    initData->boat.winchR.targetRot = boatTransform.rotation;

    transform speedBottomTransform = {};
    speedBottomTransform.location = {0.4f, 0.3f, 1.8f};
    speedBottomTransform.rotation = QuaternionIdentity();
    speedBottomTransform.scale = oneScale;

    initData->boat.speedometerBottom = 
	gameFrameworkCode->GameSpawnNewOBJ(sot_speedometer_bottom,
					   speedBottomTransform,
					   &initData->gameObjs,
					   memoryPoolCode,
					   true,
					   initData->boat.objInfo->modelMatrix);
    initData->boat.speedometerBottom->textureInfo = tl_speedometer;

    initData->boat.speedOmeter.localRollStart = -(22.0f);    
    transform speedTopTransform = {};
    speedTopTransform.location = {0.0f, 0.0f, 0.0f};
    speedTopTransform.rotation = QuaternionFromEuler(0.0f, 0.0f, initData->boat.speedOmeter.localRollStart);
    speedTopTransform.scale = oneScale;

    initData->boat.speedOmeter.movingMesh = 
	gameFrameworkCode->GameSpawnNewOBJ(sot_speedometer_top,
					   speedTopTransform,
					   &initData->gameObjs,
					   memoryPoolCode,
					   true,
					   initData->boat.speedometerBottom->modelMatrix);
    initData->boat.speedOmeter.movingMesh->textureInfo = tl_speedometer;

    initData->boat.speedOmeter.minRoll = 90.0f;
    initData->boat.speedOmeter.maxRoll = -90.0f;
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

    initData->boat.sailInfo.windRotation.eStartRotations =
	initData->boat.sailInfo.windRotation.eTargetRotations = 
	initData->boat.sailInfo.windRotation.eCurrentRotations = FromV4ToV3Rotations(GetEulerFromForwardD(initData->boat.sailInfo.windDirection));

    initData->boat.waveInfo.properties.amplitude = 0.2f;
    initData->boat.waveInfo.properties.waveRotations.eStartRotations = 
	initData->boat.waveInfo.properties.eWaveDirection = {0.0f, 90.0f, 0.0f};

    initData->boat.waveInfo.properties.waveLength = 10.0f;
    
    CalculateCameraLocation(&cameraResult, &initData->boat);
#endif    
    return(cameraResult);
}

internal m4
ApplyLocalRoll(spawned_obj_info* object, r32 roll, m4 parentM, v4 offset, v4 parentPos)
{
    object->localTransform.rotation = QuaternionFromEuler(0.0f, 0.0f, (r32)DEG2RAD(roll));
    
    object->localMatrix = CreateModelMatrix(object->localTransform.scale,
					    object->localTransform.rotation,
					    object->localTransform.location);
    v4 rotatedOffset = Vector3Transform(offset, parentM);
    parentM.e[3][0] = parentPos.x + rotatedOffset.x;
    parentM.e[3][1] = parentPos.y + rotatedOffset.y;
    parentM.e[3][2] = parentPos.z + rotatedOffset.z;
    parentM.e[3][3] = 1.0f;
    return(parentM);
}

internal m4
ComputeObjectRotationForForward(v2 localForward, v2 worldForward, v4 qParentQuat, v4 parentPos, v4 objectOffset, spawned_obj_info* affectedObject)
{
    r32 signedAngle = SignedAngleBetweenTwoVectorsDegrees(localForward, worldForward);
    v4 localRoll = QuaternionFromEuler(0.0f, 0.0f, (r32)DEG2RAD(signedAngle));
    localRoll = QuaternionNormalize(localRoll);

    m4 modelMat = MatrixRotationQuaternion(qParentQuat);
    m4 newMat = ApplyLocalRoll(affectedObject, signedAngle, modelMat, objectOffset, parentPos);
    return(affectedObject->localMatrix * newMat);    
}

internal void
ChangeCamOffset(static_cam_location newCamLocation, boat_entity* boat, game_state* gameState)
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

    if (newCamLocation != scl_center)
    {
	if (!gameState->tutorialData.movedToSides)
	    gameState->tutorialData.movedToSides = true;
    }
}

internal v4
RotateOBJ(spawned_obj_info* obj, r32 roll, r32 pitch, r32 yaw, v4 qCurrRot, r32 deltaTime, m4 parentTransform)
{
    v3 result = {};
    v4 targetQuat = QuaternionFromEuler((r32)DEG2RAD(pitch), (r32)DEG2RAD(yaw), (r32)DEG2RAD(roll));
    targetQuat = QuaternionNormalize(targetQuat);

    r32 rate = 5.0f;
    r32 slerpFactor = 0.5f - (r32)expf(-deltaTime * rate);
    if (slerpFactor > 1.0f) slerpFactor = 1.0f;

    v4 t4 = {slerpFactor, slerpFactor, slerpFactor, slerpFactor};

    
    v4 outRotation = QuaternionSlerpV(qCurrRot, targetQuat, t4);


    if (obj)
    {
	obj->modelTransform.rotation = outRotation;
	if (obj->inheritsTransform)
	{
	    obj->localMatrix = CreateModelMatrix(obj->modelTransform.scale,
						 obj->modelTransform.rotation,
						 obj->modelTransform.location);
	    obj->modelMatrix = obj->localMatrix * parentTransform;
	}
	else
	{
	    obj->modelMatrix = CreateModelMatrix(obj->modelTransform.scale,
						 obj->modelTransform.rotation,
						 obj->modelTransform.location);
	}
    }
	
    return(outRotation);
}

internal v4
RotateOBJ(spawned_obj_info* obj, v3 targetRotations, v4 qCurrRot, r32 deltaTime)
{
    m4 parentTransform = Identity();
    return(RotateOBJ(obj, targetRotations.roll, targetRotations.yaw, targetRotations.pitch, qCurrRot, deltaTime, parentTransform));
}

internal v4
RotateOBJ(spawned_obj_info* obj, v3 targetRotations, v4 qCurrRot, r32 deltaTime, m4 parentTransform)
{
    return(RotateOBJ(obj, targetRotations.roll, targetRotations.yaw, targetRotations.pitch, qCurrRot, deltaTime, parentTransform));
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
RotateOBJLocal(spawned_obj_info* objInfo, r32 deltaTime, v4 startRot, v4* targetRot, r32 lerpSpeed, v4 axis, r32 degrees, v4* offsetAngle)
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
    *offsetAngle = outRotation;
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
    boat->vOutput = qWave;
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
UpdateChildModelMatrix(spawned_obj_info* child, spawned_obj_info* parent)
{
    child->modelMatrix = child->localMatrix * parent->modelMatrix;
}

#define ROTATION_CLOCKWISE 0
#define ROTATION_COUNTERCLOCKWISE 1
#define NO_ROTATION 2

internal joystick_rotation
UpdateJoystickInformation(joystick_rotation* oldRotation, r32 x, r32 y, bool32 wasInDeadzone)
{
    joystick_rotation result = {};

    r32 angle = (r32)RAD2DEG(atan2f(y, x));
    if (angle < 0.0f)
	angle += 360.0f;
    
    
    result.angle = angle;

    result.quad = (i32)(angle / 90.0f);
    if (wasInDeadzone)
    {
	result.clockwise = NO_ROTATION;
	return(result);
    }

    r32 diff = result.angle - oldRotation->angle;

    if (diff > 180.0f) diff -= 360.0f;
    if (diff < -180.0f) diff += 360.0f;

//    r32 epsilon = 0.001f;
    r32 minAngSpeed = 1.0f;
    if (diff < -minAngSpeed)
    {
	result.clockwise = ROTATION_CLOCKWISE;
    }
    else if (diff > minAngSpeed)
    {
	result.clockwise = ROTATION_COUNTERCLOCKWISE;
    }
    else
    {
	result.clockwise = NO_ROTATION;
    }
    return(result);
}

r32 internal
InterpretControllerInformation(game_controller_input* pad, bool32 rotateClockwise, boat_entity* boat, joystick_rotation* newJoystick, joystick_rotation* oldJoystick, r32 deltaTime, winch* rotatingWinch, r32 inputYaw)
{
    r32 newYaw = inputYaw;
    sail_type* main = &boat->sailInfo.mainSail;
    r32 sailDeg = rotateClockwise ? -SAILMOVEMENTSENS : SAILMOVEMENTSENS;
    r32 winchDeg = rotateClockwise ? -0.5f : 0.5f;
    v4 yAxis = {0.0f, 1.0f, 0.0f, 0.0f};

    r32 x = pad->leftStickAverageX;
    r32 y = pad->leftStickAverageY;
    r32 stickMagSq = (x * x) + (y * y);
    r32 deadzoneSq = 0.2f * 0.2f;
    

    if (stickMagSq > deadzoneSq)
    {
	bool32 wasInDeadzone = (oldJoystick->clockwise == NO_ROTATION && oldJoystick->angle == 0.0f && oldJoystick->quad == 0);
	*newJoystick = UpdateJoystickInformation(oldJoystick,
						 x, y, wasInDeadzone);
				

	if ((newJoystick->clockwise == rotateClockwise))
	{
	    newYaw = inputYaw + sailDeg;

	    rotatingWinch->currRot = RotateOBJ(rotatingWinch->winchModel,
				       deltaTime,
				       rotatingWinch->startRot,
				       &rotatingWinch->targetRot,
				       boat->lerpTimeSpeed,
				       yAxis,
				       winchDeg,
				       boat->objInfo->modelMatrix);

	}

			

    }
    else
    {
	newJoystick->clockwise = NO_ROTATION;
	newJoystick->angle = 0.0f;
	newJoystick->quad = 0;
    }
    *oldJoystick = *newJoystick;
    
    return(newYaw);
}

internal void
RotateSpeedOMeter(boat_entity* boat)
{
    r32 normalizedSpeed = 0.0f;
    if (boat->movementSpeed >= 0.0f)
	normalizedSpeed = (boat->movementSpeed - boat->bottomSpeed) / (boat->topSpeed - boat->bottomSpeed);

    v4 offset = {0.4f, 0.3f, 1.8f, 0.0f};

    r32 newRoll = Lerp(boat->speedOmeter.minRoll, boat->speedOmeter.maxRoll, normalizedSpeed);

    v4 normalBoatRot = QuaternionNormalize(boat->objInfo->modelTransform.rotation);    
    m4 boatMat = MatrixRotationQuaternion(normalBoatRot);

    m4 newMat = ApplyLocalRoll(boat->speedOmeter.movingMesh,
			       newRoll,
			       boatMat,
			       offset,
			       boat->objInfo->modelTransform.location);
    
    boat->speedOmeter.movingMesh->modelMatrix = boat->speedOmeter.movingMesh->localMatrix * newMat;
}

internal void
DebugInputs(game_controller_input* keyboard, game_controller_input* gamePad, boat_entity* boat)
{
    if (keyboard->two.started)
    {
	boat->sailInfo.windDirection = {1.0f, 0.0f, 0.0f, 0.0f};
	keyboard->two.started = false;
    }
    if (keyboard->three.started)
    {
	boat->sailInfo.windDirection = {0.0f, 0.0f, 1.0f, 0.0f};
	keyboard->three.started = false;
    }
    if (keyboard->four.started)
    {
	boat->sailInfo.windDirection = {-1.0f, 0.0f, 0.0f, 0.0f};
	keyboard->four.started = false;
    }
    if (keyboard->five.started)
    {
	boat->sailInfo.windDirection = {0.0f, 0.0f, -1.0f, 0.0f};	
	keyboard->five.started = false;
    }
}

#include "sail_data_collection.cpp"

extern "C" SAIL_UPDATE(SailUpdate)
{
    //Update our input
    //Update our camera

    game_controller_input* controller = GetController(input, 0);    
    game_controller_input* padController = GetController(input, 1);
    Assert(padController);
    boat_entity* boat = &initData->boat; 

    if (controller || padController)
    {
	if (controller->one.started)
	{
	    initData->isFreeCam = !initData->isFreeCam;
	    controller->one.started = false;
	}
	if (initData->isFreeCam)
	{
	    r32 camVelocity = camera->movementSpeed * deltaTime;
	    if (controller->moveForward.endedDown || padController->moveForward.endedDown)
	    {
		//w
		camera->position = camera->position + (camera->front * camVelocity);
	    }

	    if (controller->moveLeft.endedDown || padController->moveForward.endedDown)
	    {
		//a
		camera->position = camera->position - (camera->right * camVelocity);
	    }

	    if (controller->moveBackward.endedDown || padController->moveForward.endedDown)
	    {
		//s
		camera->position = camera->position - (camera->front * camVelocity);	    
	    }

	    if (controller->moveRight.endedDown || padController->moveForward.endedDown)
	    {
		//d
		camera->position = camera->position + (camera->right * camVelocity);
	    }
	}
	else
	{
	    sail_type* main = &boat->sailInfo.mainSail;

	    r32 velocity = boat->movementSpeed * deltaTime;

	    wind_sock* windSock = &boat->windSock;

	    DebugInputs(controller, padController, boat);
	    v4 newLocation = {};
	    if (boat->movementDirection == bmd_forward)
	    {
		newLocation = boat->objInfo->modelTransform.location + (boat->forward * velocity);
	    }
	    else
	    {
		v4 waveDir = GetForwardVector((r32)DEG2RAD(boat->waveInfo.properties.eWaveDirection.pitch),
					      (r32)DEG2RAD(boat->waveInfo.properties.eWaveDirection.yaw));
#if 0
		v4 waveDir =
		    {
			boat->waveInfo.properties.waveDirection.x,
			boat->waveInfo.properties.waveDirection.y,
			boat->waveInfo.properties.waveDirection.z,
			0.0f
		    };
#endif		
		newLocation = boat->objInfo->modelTransform.location + (waveDir * velocity);
	    }

	    boat->objInfo->modelTransform.location = boat->objInfo->modelTransform.location + (boat->forward * velocity);	    
	    boat->objInfo->modelMatrix = CreateModelMatrix(boat->objInfo->modelTransform.scale,
							   boat->objInfo->modelTransform.rotation,
							   boat->objInfo->modelTransform.location);


	    CalculateCameraLocation(camera, boat);
	    r32 speedCheck = boat->movementSpeed > (boat->topSpeed - 1.0f);
	    if ((!gameState->tutorialData.upToSpeed) && (speedCheck))
	    {
		gameState->tutorialData.upToSpeed = true;
	    }
	    else if ((gameState->tutorialData.upToSpeed) && !speedCheck)
	    {
		gameState->tutorialData.upToSpeed = false;
	    }
		
	    
	    
	    if ((controller->moveDown.started) || (padController->five.halfTransitionCount == 1 && padController->five.endedDown))
	    {
		ChangeCamOffset(static_cam_location::scl_left, boat, gameState);
		CalculateCameraLocation(camera, boat);
		controller->moveDown.started = false;
	    }

	    //E
	    if ((controller->moveUp.started) || (padController->six.halfTransitionCount == 1 && padController->six.endedDown))
	    {
		ChangeCamOffset(static_cam_location::scl_right, boat, gameState);
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

	    

	    if ((padController->lookRight.endedDown) || (padController->lookLeft.endedDown) ||
		(padController->lookUp.endedDown) || (padController->lookDown.endedDown))
	    {
		camera->xChange = deltaTime * (-padController->rightStickAverageX * PADSENS);
		camera->yChange = deltaTime * (padController->rightStickAverageY * PADSENS);
	    }

	    r32 turnSpeed = 0.0f;
	    if (boat->movementDirection == bmd_forward)
	    {
		turnSpeed = (40.0f * boat->movementSpeed) * deltaTime;
	    }
	    else
	    {
		turnSpeed = 0.5f;
	    }
	    r32 turnSlerp = 10.0f;
	    r32 currYaw = boat->targetBoatRotations.yaw;

	    r32 turnSailSlerp = 5.0f;
	    r32 sailTurnSpeed = 0.08f;
	    r32 currSailYaw = main->targetSailRotations.yaw;	    
	    if (boat->boatCameraMode == bcm_steer)
	    {
		if (controller->moveLeft.endedDown || padController->moveLeft.endedDown)
		{

		    boat->currTargetYaw = boat->targetBoatRotations.y + turnSpeed;
		}

		
		if (controller->moveRight.endedDown || padController->moveRight.endedDown)
		{
		    boat->currTargetYaw = boat->targetBoatRotations.y - turnSpeed;
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

		r32 sailDeg = 0.5f;
		if (boat->staticCamLocation == static_cam_location::scl_left)
		{

		    main->targetYaw = InterpretControllerInformation(padController,
								     ROTATION_COUNTERCLOCKWISE,
								     boat,
								     &initData->newJoystick,
								     &initData->oldJoystick,
								     deltaTime,
								     &boat->winchL,
								     main->targetSailRotations.y);

		    if (controller->moveLeft.endedDown)
		    {
			main->targetYaw = main->targetSailRotations.y - sailTurnSpeed;
		    }
		}
		else if (boat->staticCamLocation == static_cam_location::scl_right)
		{
		    main->targetYaw = InterpretControllerInformation(padController,
								     ROTATION_CLOCKWISE,
								     boat,
								     &initData->newJoystick,
								     &initData->oldJoystick,
								     deltaTime,
								     &boat->winchR,
								     main->targetSailRotations.y);

		    if (controller->moveRight.endedDown)
		    {
			main->targetYaw = main->targetSailRotations.y + sailTurnSpeed;
		    }
		}
	    }


	    boat->targetBoatRotations.yaw = Lerp(currYaw, boat->currTargetYaw, 1.0f - expf(-deltaTime * turnSlerp));


//	    main->targetSailRotations.yaw = Lerp(currSailYaw, main->targetYaw, 1.0f - expf(-deltaTime * turnSailSlerp));
	    main->targetSailRotations.yaw = main->targetYaw;


	    v4 targetSailRot = QuaternionFromEuler(main->targetSailRotations.pitch,
						   main->targetSailRotations.yaw,
						   main->targetSailRotations.roll);
	    v4 t4 = {0.3f, 0.3f, 0.3f, 0.3f};
	    boat->mast->localTransform.rotation = targetSailRot;
	    boat->mast->localMatrix = CreateModelMatrix(boat->mast->localTransform.scale,
							boat->mast->localTransform.rotation,
							boat->mast->localTransform.location);
	    
	    AddNoiseToWaves(&boat->waveInfo.properties);
	    ApplyBoatWaveRotations(boat, deltaTime);	

	    UpdateBoatVectors(boat);


	    CalculateCameraLocation(camera, boat);




	    //Speeeeeeeeeed is key brother (speedometer code)
	    RotateSpeedOMeter(boat);

	    
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


	    //Wind model


	    v2 boatXZ = {boat->forward.x, boat->forward.z};
	    v2 north = {0.0f, 1.0f};
	    v2 windXZ = {clearWind.x, clearWind.z};
	    v4 windModelOffset = {-0.4f, 0.3f, 1.8f, 0.0f};	    

	    //boat forward and world forward
	    boat->windModelTop->modelMatrix = ComputeObjectRotationForForward(boatXZ,
									      north,
									      normalBoatRot,
									      boatPos,
									      windModelOffset,
									      boat->windModelTop);
	    //wind forward and world forward, this only needs to change when the wind changes
	    boat->windModelBottom->modelMatrix = ComputeObjectRotationForForward(windXZ,
										 north,
										 normalBoatRot,
										 boatPos,
										 windModelOffset,
										 boat->windModelBottom);

	    
	    
	    UpdateChildModelMatrix(boat->windCardinal, boat->objInfo);

	    UpdateChildModelMatrix(boat->winchR.winchModel, boat->objInfo);
	    UpdateChildModelMatrix(boat->winchL.winchModel, boat->objInfo);
	    UpdateChildModelMatrix(boat->speedometerBottom, boat->objInfo);
	    UpdateChildModelMatrix(boat->mast, boat->objInfo);
	}
    }

    gameFrameworkCode->GameUpdateCamera(camera);
    RunPlayerTutorial(gameState, &boat->sailInfo);

//    UpdateWindDirection(&boat->sailInfo.windRotation, &boat->sailInfo.windDirection, deltaTime);


}
