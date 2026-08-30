#if !defined(SAIL_GAME_LAYER_H)
#include "D:/ExternalCustomAPIs/Types/typedefs.h"
#include "D:/ExternalCustomAPIs/Game/code/game_framework_dll_include.h"
#include "D:/ExternalCustomAPIs/MemoryPools/code/memory_pool_dll_include.h"
#include "D:/ExternalCustomAPIs/OBJLoader/code/obj_parser_dll_include.h"

#if defined(_MSC_VER)
#define GAME_CALL __vectorcall
#elif defined(__clang__) || defined(__GNUC__)
#define GAME_CALL __attribute__((vectorcall))
#else
#define GAME_CALL
#endif

enum sail_orientation
{
    so_closeHauled = 0,
    so_closeReach = 22,
    so_beamReach = 45,
    so_broadReach = 67,
    so_running = 90,
};

enum tack_orientation
{
    to_starboard,
    to_port
};

struct angle_comparison
{
    r32 currentAngle;
    r32 previousAngle;
};

struct sail_type
{
    tack_orientation tackOrientation;
    sail_orientation sailOrientation;

    v4 qSailRot;
    v4 qTargetRot;
    v4 sailForward;
    v4 startRot;

    v4 locationOffset;
    
    r32 pitch;
    r32 yaw;
    
    angle_comparison sailToBoat;
};

struct sailing
{
    v4 windDirection;
    r32 windSpeed;
    sail_type mainSail;
};

enum static_cam_location
{
    scl_center,
    scl_left,
    scl_right
};

enum boat_cam_mode
{
    bcm_steer,
    bcm_winch
};

struct wind_sock
{
    spawned_obj_info* model;
    v4 startRot;
    v4 currRot;
    v4 targetRot;
};

struct sine_wave_properties
{
    r32 amplitude;
    r32 period;
    r32 horizontalShift;
    r32 verticalShift;

    r32 sineCrest;
    r32 sineTrough;
};

struct wave
{
    r32 t;
    r32 waveSpeed;
    v2 currentPoint;
    v2 tangentPoint;
    v2 normalPoint;
    sine_wave_properties waveProperties;

    v4 maxRotD;
    v4 minRotD;
    v4 qWaveTilt;
};

struct boat_entity
{

    boat_cam_mode boatCameraMode;
//Quaternions
    v4 qTargetRot;
    v4 currRot;
    v4 startRot;


    //Vectors
    v4 forward;
    v4 up;
    v4 right;
    
    wind_sock windSock;

    r32 windAngle;
    r32 sailAngle;

    r32 boatToSail;
    
    r32 pitch;
    r32 yaw;
    r32 roll;

    v3 targetBoatRotations;
    
    r32 currRotTime;
    r32 lerpTimeSpeed;

    spawned_obj_info* objInfo;
    spawned_obj_info* mast;
    spawned_obj_info* windModelBottom;
    spawned_obj_info* windModelTop;

    i32 flag;

    bool32 isRotating;
    static_cam_location staticCamLocation;

    v4 centerCamOffset;
    v4 leftCamOffset;
    v4 rightCamOffset;
    v4 currCamOffset;

    r32 topSpeed;
    r32 bottomSpeed;
    r32 movementSpeed;

    sailing sailInfo;

    wave waveInfo;
};

struct sail_initialize_data
{
    game_loaded_objs gameObjs;

    game_loaded_textures gameTextures;
    boat_entity boat;
    bool32 isFreeCam;
};

struct platform_info
{
    v2 aspect;
    framework_arenas frameworkArenas; //Assign in win32 layer
    parse_obj_data_code* parseObjCode; //assign this as well

    
};

    
struct inherited_location_info
{
    v4 inheritedRotation;
    v4 targetForward;
    v4 position;
};


inline v4
GetForwardVector(r32 pitch, r32 yaw)
{
    v4 result =
    {
	(r32)(cosf(pitch) * sin(yaw)),
	(r32)(sinf(pitch)),
	(r32)(cosf(yaw) * cosf(pitch))
    };

    result = NormalizeV3(result);
    return(result);
}


#define SAIL_UPDATE(name) void GAME_CALL name(game_framework_dll_code* gameFrameworkCode, memory_pool_dll_code* memoryPoolCode, game_input* input, game_camera* camera, r32 deltaTime, sail_initialize_data* initData)
typedef SAIL_UPDATE(sail_update);

//replace sail_initialize_data w/ game_camera, put sail_initialize_data as pointer and make it a magic function
//otherwise we have to worry about everything in this sail_intialize_data struct to be alignas(16) and that's too
//much work
#define SAIL_INITIALIZE(name) game_camera GAME_CALL name(sail_initialize_data* initData, game_framework_dll_code* gameFrameworkCode, memory_pool_dll_code* memoryPoolCode, platform_info* platformInfo, program_memory* pgMem)
typedef SAIL_INITIALIZE(sail_initialize);

#define SAIL_GAME_LAYER_H
#endif
