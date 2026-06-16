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

struct sail_initialize_data
{
    game_loaded_objs gameObjs;
};

struct platform_info
{
    v2 aspect;
    framework_arenas frameworkArenas; //Assign in win32 layer
    parse_obj_data_code* parseObjCode; //assign this as well

    
};

#define SAIL_UPDATE(name) void GAME_CALL name(game_framework_dll_code* gameFrameworkCode, memory_pool_dll_code* memoryPoolCode, game_input* input, game_camera* camera, r32 deltaTime)
typedef SAIL_UPDATE(sail_update);

//replace sail_initialize_data w/ game_camera, put sail_initialize_data as pointer and make it a magic function
//otherwise we have to worry about everything in this sail_intialize_data struct to be alignas(16) and that's too
//much work
#define SAIL_INITIALIZE(name) game_camera GAME_CALL name(sail_initialize_data* initData, game_framework_dll_code* gameFrameworkCode, memory_pool_dll_code* memoryPoolCode, platform_info* platformInfo, program_memory* pgMem)
typedef SAIL_INITIALIZE(sail_initialize);

#define SAIL_GAME_LAYER_H
#endif
