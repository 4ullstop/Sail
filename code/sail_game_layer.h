#if !defined(SAIL_GAME_LAYER_H)
#include "D:/ExternalCustomAPIs/Types/typedefs.h"
#include "D:/ExternalCustomAPIs/Game/code/game_framework_dll_include.h"
#include "D:/ExternalCustomAPIs/MemoryPools/code/memory_pool_dll_include.h"
#include "D:/ExternalCustomAPIs/OBJLoader/code/obj_parser_dll_include.h"
#include "sail_data_collection.h"
#include "scripted_events.h"

#if defined(_MSC_VER)
#define GAME_CALL __vectorcall
#elif defined(__clang__) || defined(__GNUC__)
#define GAME_CALL __attribute__((vectorcall))
#else
#define GAME_CALL
#endif

enum e_weather_level : u8
{
    ewl_1 = 2,
    ewl_2 = 4,
    ewl_3 = 6,
    ewl_4 = 8,
};

struct game_state
{
    tutorial_data tutorialData;
    r32 msPerFrame;
    u8 weatherLevel;
    u8 newWeatherLevel;
};

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

    v4 rotOffset;
    
    r32 pitch;
    r32 yaw;
    r32 roll;

    r32 targetYaw;

    r32 localYaw;
    r32 localPitch;

    v3 targetSailRotations;
    
    angle_comparison sailToBoat;
};

struct rotational_update
{
    v3 eTargetRotations;
    v3 eCurrentRotations;
    v3 eStartRotations;
    bool32 inRotation;
    r32 currentLerp;
    r32 lerpSpeed;

    v4 vCurrent;
    v4 qCurrent;

    bool32 resetStartRotation;
};

struct sailing
{
    v4 windDirection;
    rotational_update windRotation; //put this in when finished refactoring
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

struct timer
{
    r32 time;
    bool32 running;
    r32 endTime;
};

struct wind_sock
{
    spawned_obj_info* model;
    v4 startRot;
    v4 currRot;
    v4 targetRot;
};

struct lerp_update
{
    r32 value;
    r32 lerpP; //lerp percent
    bool32 lerpRunning;
};

struct real_update
{
    r32 target;
    r32 start;

    lerp_update updateInfo;
};

struct wave_component
{
    r32 amplitude;
    r32 waveLength;
    r32 angleDegrees;
    r32 speed;
};

struct wave_properties
{

    u32 waveCount;
    wave_component components[8];
    r32 averageWaveDir;
};

struct wave
{
    r32 t;
    r32 waveSpeed;
    v2 currentPoint;
    v2 tangentPoint;
    v2 normalPoint;
    wave_properties properties;
    wave_properties waveMinMaxProperties;

};

struct winch
{
    spawned_obj_info* winchModel;
    
    v4 targetRot;
    v4 currRot;
    v4 startRot;
};

struct speedometer
{
    spawned_obj_info* movingMesh;

    r32 localRollStart;
    r32 currRoll;
    r32 maxRoll;
    r32 minRoll;
};

enum boat_movement_direction
{
    bmd_forward,
    bmd_waves,
};

struct boat_entity
{
    boat_movement_direction movementDirection;
    
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
    v3 angularVelocity;

    r32 currTargetYaw;
    
    r32 currRotTime;
    r32 lerpTimeSpeed;

    spawned_obj_info* objInfo;
    spawned_obj_info* mast;
    spawned_obj_info* windModelBottom;
    spawned_obj_info* windModelTop;
    spawned_obj_info* windCardinal;


    spawned_obj_info* speedometerBottom;
    speedometer speedOmeter;
    winch winchL;
    winch winchR;
    
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

    i32 output;
    r32 rOutput;
    v4 vOutput;
};


struct joystick_rotation
{
    v2 stickAverage;

    r32 angle;
    bool32 clockwise; //true==clockwise false==counterclockwise
    i32 quad;
};

struct sail_initialize_data
{
    game_loaded_objs gameObjs;

    game_loaded_textures gameTextures;
    boat_entity boat;
    bool32 isFreeCam;

    joystick_rotation newJoystick;
    joystick_rotation oldJoystick;
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

#define SAIL_UPDATE(name) void GAME_CALL name(game_framework_dll_code* gameFrameworkCode, memory_pool_dll_code* memoryPoolCode, game_input* input, game_camera* camera, r32 gameDeltaTime, sail_initialize_data* initData, game_state* gameState)
typedef SAIL_UPDATE(sail_update);

//replace sail_initialize_data w/ game_camera, put sail_initialize_data as pointer and make it a magic function
//otherwise we have to worry about everything in this sail_intialize_data struct to be alignas(16) and that's too
//much work
#define SAIL_INITIALIZE(name) game_camera GAME_CALL name(sail_initialize_data* initData, game_framework_dll_code* gameFrameworkCode, memory_pool_dll_code* memoryPoolCode, platform_info* platformInfo, program_memory* pgMem)
typedef SAIL_INITIALIZE(sail_initialize);

#define SAIL_GAME_LAYER_H
#endif
