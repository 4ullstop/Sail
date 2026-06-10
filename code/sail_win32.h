#if !defined(SAIL_WIN32_H)
#include "D:/ExternalCustomAPIs/Types/direct_x_typedefs.h"
#include "D:/ExternalCustomAPIs/Math/forty_math_fast.h"
#include "D:/ExternalCustomAPIs/MemoryPools/code/memory_pool_dll_include.h"

struct win32_arenas
{
    memory_arena InitArena;
    memory_arena perFrameArena;
    
    u8* arenaBase;
};


//Dummy struct for testing purposes

#if 0
struct dx_camera
{
    constant_buffer_struct constantBufferData;
    
    DirectX::XMVECTOR position;
    DirectX::XMVECTOR right;
    DirectX::XMVECTOR worldUp;
    DirectX::XMVECTOR up;
    DirectX::XMVECTOR front;
    r32 yaw, pitch, movementSpeed, turnSpeed;

    
    DirectX::XMVECTOR startEye;
    DirectX::XMVECTOR startAt;
    DirectX::XMVECTOR startUp;

    r32 fovY;

    v2 aspect;
    
    r32 targetZoom, currZoom, lag, zoomingTo;
    DirectX::XMMATRIX viewInverted;
    DirectX::XMVECTOR targetPos;
    DirectX::XMVECTOR positionTo;
    
    DirectX::XMVECTOR targetQRot;
    DirectX::XMVECTOR qRotationTo;
    DirectX::XMVECTOR currQRot;
    
    DirectX::XMVECTOR viewCenter;
    DirectX::XMVECTOR eye;
    DirectX::XMVECTOR upDir;
};

#endif
//

struct shaders
{
    ID3D11VertexShader* vertexShader;
    ID3D11Buffer* vsConstantBuffer;
    ID3D11PixelShader* pixelShader;

    ID3D11InputLayout* vertexInputLayout;
    
};

#define SAIL_WIN32_H
#endif
