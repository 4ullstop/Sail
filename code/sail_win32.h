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

    ID3D11VertexShader* uiVertexShader;
    ID3D11PixelShader* uiPixelShader;

    ID3D11InputLayout* uiInputLayout;


    ID3D11VertexShader* oceanVSShader;
    ID3D11InputLayout* oceanInputLayout;
    ID3D11PixelShader* oceanPSShader;

    ID3D11VertexShader* ppVS;
    ID3D11InputLayout* ppIALayout;
    ID3D11PixelShader* ppPS;
};

struct alignas(16) ocean_sine_constant
{
    r32 amp[4][4];
    r32 frequency[4][4];
    r32 phase[4][4];
    r32 directions[4][4];
};

struct global_lighting
{
    DirectX::XMVECTOR lightNormal;
    DirectX::XMVECTOR location;
};

struct ocean_buffers
{
    ID3D11Buffer* vertBuffer;
    ID3D11Buffer* indexBuffer;

    i32 indexCount;
    ID3D11Buffer* oceanUpdateBuffer;    
    ID3D11Buffer* oceanSineConstants;

    ocean_sine_constant oceanSC;

    ID3D11Buffer* oceanLightingBuffer;
};

struct object_constants
{
//    DirectX::XMFLOAT4 worldPos;
    DirectX::XMFLOAT4X4 modelMat;
};

struct alignas(16) ocean_update 
{
    DirectX::XMFLOAT4X4 modelMat;
    r32 t;
};

struct material_constants
{
    DirectX::XMFLOAT4 hasMaterials;
};

struct sail_constant_buffers
{
    ID3D11Buffer* dynamicVBuffer;
    ID3D11Buffer* dynamicPBuffer;

};

#define SAIL_WIN32_H
#endif
