#include "sail_win32.h"

#include "sail_game_layer.h"
#include <windows.h>
#include <stdio.h>

#include "D:/ExternalCustomAPIs/FileReader/file_reader.h"
#include "D:/ExternalCustomAPIs/FileReader/file_reader.cpp"

#include "D:/ExternalCustomAPIs/Win32/code/win32_framework_dll_include.h"


#include <DirectXMath.h>

#include <d3d11_2.h>
#include <dxgi1_6.h>



global_variable ID3D11Device* d3dDevice;
global_variable ID3D11DeviceContext* context;
global_variable IDXGISwapChain1* swapChain;
global_variable ID3D11Texture2D* backBuffer;
global_variable ID3D11RenderTargetView* renderTargetView;
global_variable ID3D11DepthStencilView* depthStencilView;
global_variable ID3D11Texture2D* depthStencil;
global_variable D3D11_TEXTURE2D_DESC bbDesc;

global_variable r32 screenWidth = 1280;
global_variable r32 screenHeight = 720;


global_variable memory_pool_dll_code memoryPoolCode;
global_variable win32_arenas* programArenas;

global_variable game_framework_dll_code gameFrameworkCode;
global_variable win32_framework_dll_code win32Code;
global_variable parse_obj_data_code parseObjCode;

global_variable v2 aspect = v2{(r32)1.7075098752975464, (r32)0.960474312305450};

global_variable thread_context blankThread;

global_variable program_state programState;
global_variable i64 perfCountFrequency;

inline LARGE_INTEGER
Sail32GetWallClock(void)
{
    LARGE_INTEGER result = {};
    QueryPerformanceCounter(&result);
    return(result);
}

internal r32
Sail32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    r32 result = ((r32)(end.QuadPart - start.QuadPart) / (r32)perfCountFrequency);
    return(result);
}

internal void
DXTestViewAndPerspective(dx_camera* camera)
{

    DirectX::XMMATRIX startingViewMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtRH(camera->startEye,
												camera->startAt,
												camera->startUp));

    
    DirectX::XMStoreFloat4x4(&camera->constantBufferData.view, startingViewMatrix);

    camera->fovY = 2.0f * (r32)(atan(tan(DirectX::XMConvertToRadians(70) * 0.5f)) / aspect.y);

    camera->constantBufferData.world = {};
    
    DirectX::XMStoreFloat4x4(
	&camera->constantBufferData.projection,
	DirectX::XMMatrixTranspose(
	    DirectX::XMMatrixPerspectiveFovRH(
		camera->fovY,
		aspect.x,
		0.01f,
		1000.0f)
	    )
	);

}

internal void
DXUpdateCam(dx_camera* camera)
{
    camera->front =
	{
	    (r32)(cos(camera->pitch) * sin(camera->yaw)),
	    (r32)(sin(camera->pitch)),
	    (r32)(cos(camera->yaw) * cos(camera->pitch)),	    	    
	};

    //Normalize the magnitude

    camera->front = DirectX::XMVector3Normalize(camera->front);

    camera->right = DirectX::XMVector4Normalize(DirectX::XMVector3Cross(camera->front, camera->worldUp));
    camera->up = DirectX::XMVector4Normalize(DirectX::XMVector3Cross(camera->right, camera->front));

    //Update our view matrix
    DirectX::XMStoreFloat4x4(
	&camera->constantBufferData.view,
	DirectX::XMMatrixTranspose(
	    DirectX::XMMatrixLookAtRH(
		camera->position,
		DirectX::XMVectorAdd(camera->front, camera->position),
		camera->up)
	    )
	);
    
}

LRESULT CALLBACK Win32MainWindowProc(HWND hwnd,
				     UINT uMsg,
				     WPARAM wParam,
				     LPARAM lParam)
{
    LRESULT result = 0;
    switch(uMsg)
    {
    case WM_ACTIVATEAPP:
    {
	OutputDebugString("App activated\n");
    } break;
    case WM_CLOSE:
    case WM_DESTROY:
    case WM_QUIT:
    {
	programState.running = false;
    } break;
    default:
    {
	result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    } break;
    }

    return(result);
}


internal void
CreateShaders(shaders* gameShaders)
{
    HRESULT hr = {};

    FILE* vShader, *pShader;
    BYTE* bytes = 0;

    size_t destSize = 4096;
    size_t bytesRead = 0;

    bytes = (BYTE*)memoryPoolCode.PushStruct(&programArenas->InitArena, sizeof(bytes));

    debug_read_file_result fileResult = DEBUGPlatformReadEntireFile(&blankThread, "../build/vs.cso");

    bytes = (BYTE*)fileResult.contents;
    hr = d3dDevice->CreateVertexShader(fileResult.contents,
				       fileResult.contentsSize,
				       nullptr,
				       &gameShaders->vertexShader);

    D3D11_INPUT_ELEMENT_DESC iaDesc[] =
    {
	{
	    "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
	    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0
	},

	{
	    "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT,
	    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0
	},
    };

    hr = d3dDevice->CreateInputLayout(
	iaDesc,
	ArrayCount(iaDesc),
	bytes,
	fileResult.contentsSize,
	&gameShaders->vertexInputLayout);

    debug_read_file_result pixelShaderResult = DEBUGPlatformReadEntireFile(&blankThread, "../build/ps.cso");

    bytes = (BYTE*)pixelShaderResult.contents;
    hr = d3dDevice->CreatePixelShader(
	pixelShaderResult.contents,
	pixelShaderResult.contentsSize,
	nullptr,
	&gameShaders->pixelShader);

    //Standard constant buffer
    CD3D11_BUFFER_DESC cbDesc(
	sizeof(constant_buffer_struct),
	D3D11_BIND_CONSTANT_BUFFER);

    hr = d3dDevice->CreateBuffer(
	&cbDesc,
	nullptr,
	&gameShaders->vsConstantBuffer);
}

internal draw_buffers*
GetDrawBuffersFromSpawnable(win32_spawnable_objs* win32Objs, spawned_obj_info* info)
{
    draw_buffers* result = 0;
    result = &win32Objs->objDrawnBuffers[info->type];
    return(result);
}

internal void
Render(game_loaded_objs* gameObjs, win32_spawnable_objs* win32Objs, sail_constant_buffers* cBuffers, shaders* shader, dx_camera* dxCam)
{
    //Normal render setup
    r32 teal[] = {0.098f, 0.439f, 1.000f};

    context->UpdateSubresource(shader->vsConstantBuffer, 0, nullptr, &dxCam->constantBufferData, 0, 0);

    context->ClearRenderTargetView(renderTargetView,
				   teal);

    context->ClearDepthStencilView(depthStencilView,
				   D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
				   1.0f,
				   0);
    
    context->OMSetRenderTargets(1,
				&renderTargetView,
				depthStencilView);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(shader->vertexInputLayout);
    context->VSSetConstantBuffers(0, 1, &shader->vsConstantBuffer);


    context->VSSetShader(shader->vertexShader,
			 nullptr,
			 0);

    context->PSSetShader(shader->pixelShader,
			 nullptr,
			 0);
    
    //Just rendering the small things we want to render
    listed_memory_node* objNode = (listed_memory_node*)gameObjs->spawnedObjNodes;

    HRESULT hr = {};
    for (i32 i = 0; i < gameObjs->spawnedObjMemory->numOfItems; i++)
    {
	Assert(objNode);

	spawned_obj_info* objInfo = (spawned_obj_info*)objNode->data;
	draw_buffers* drawBuffers = GetDrawBuffersFromSpawnable(win32Objs, objInfo);
	context->VSSetConstantBuffers(1, 1, &cBuffers->dynamicVBuffer);

	UINT stride = sizeof(vertex_position_color);
	UINT offset = 0;

	context->IASetVertexBuffers(0, 1, &drawBuffers->vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(drawBuffers->indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	//How we set it up:
	/*
	  Convert all transformations (scale, rotation, location) into matrices
	  Multiply them together scaleM * rotation * location == ModelMatrix
	  Upload model matrix as opposed to Float4 to shader to be used
	 */
	D3D11_MAPPED_SUBRESOURCE mapped;
	hr = context->Map(cBuffers->dynamicVBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	object_constants* data = (object_constants*)mapped.pData;

	
	DirectX::XMMATRIX dxMat = win32Code.Win32FromM4ToXMMATRIX(objInfo->modelMatrix);


	DirectX::XMStoreFloat4x4(&data->modelMat, dxMat);

	context->Unmap(cBuffers->dynamicVBuffer, 0);

	context->DrawIndexed(
	    drawBuffers->indexCount,
	    0,
	    0);

	objNode = objNode->next;
    }
}

int CALLBACK WinMain(HINSTANCE hInstance,
		     HINSTANCE hPrevInstance,
		     LPSTR lpCmdLine,
		     int nCmdShow)
{

    /*
      Testing zone
     */

    v4 v = {2.234f, 51.4357893f, 34.12548932f, 2904.239834f};
    DirectX::XMVECTOR xmV = DirectX::XMVectorSet(2.234f, 51.4357893f, 34.12548932f, 2904.239834f);    
#if 0
    //v4

    v4 fmRound = VectorRound(v);

    //XMVECTOR

    DirectX::XMVECTOR xmRound = DirectX::XMVectorRound(xmV);

    v4 fmModAng = VectorModAngles(v);

    DirectX::XMVECTOR xmModAng = DirectX::XMVectorModAngles(xmV);



    m4 m = {};
    m.r[0] = v;
    m.r[1] = v;
    m.r[2] = v;
    m.r[3] = v;    

    m4 fmQuat = MatrixRotationQuaternion(v);

    DirectX::XMMATRIX xmQuat = DirectX::XMMatrixRotationQuaternion(xmV);


    
    v4 fmRN = QuaternionRotationNormal(v, 30.f);

    DirectX::XMVECTOR xmRN = DirectX::XMQuaternionRotationNormal(xmV, 30.f);
#endif

    


    
    UINT desiredSchedulerMs = 1;
    bool32 sleepIsGranular = (timeBeginPeriod(desiredSchedulerMs) == TIMERR_NOERROR);


//Used modules loading and function loading

    

    
    HMODULE gameFrameworkLibrary = LoadLibrary("D:/ExternalCustomAPIs/Game/dll/game_framework.dll");


    
    if (gameFrameworkLibrary)
    {
	gameFrameworkCode.GameCreateViewAndPerspective = (game_create_view_and_perspective*)GetProcAddress(gameFrameworkLibrary, "CreateViewAndPerspective");
	gameFrameworkCode.GameUpdateCamera = (game_update_camera*)GetProcAddress(gameFrameworkLibrary, "GameUpdateCamera");
	gameFrameworkCode.GameLoadOBJFiles = (game_load_obj_files*)GetProcAddress(gameFrameworkLibrary, "LoadGameOBJFiles");
	gameFrameworkCode.GameSpawnNewOBJ = (game_spawn_new_obj*)GetProcAddress(gameFrameworkLibrary, "SpawnNewOBJ");
    }

    HMODULE memoryPoolLibrary = LoadLibrary("D:/ExternalCustomAPIs/MemoryPools/dll/memory_pools.dll");


    //for time being only using the ones I need, remember to initialize them if you decide you need things
    //like listed nodes
    if (memoryPoolLibrary)
    {
	memoryPoolCode.PushStruct = (memory_pool_push_struct*)GetProcAddress(memoryPoolLibrary, "PushStruct");
	memoryPoolCode.PushArray = (memory_pool_push_array*)GetProcAddress(memoryPoolLibrary, "PushArray");
	memoryPoolCode.PoolAlloc = (memory_pool_alloc*)GetProcAddress(memoryPoolLibrary, "PoolAlloc");
	memoryPoolCode.InitArena = (memory_pool_initialize_arena*)GetProcAddress(memoryPoolLibrary, "InitializeArena");
	memoryPoolCode.ClearArena = (memory_pool_clear_arena*)GetProcAddress(memoryPoolLibrary, "ClearArena");
	memoryPoolCode.PushArraySized = (memory_pool_push_array_sized*)GetProcAddress(memoryPoolLibrary, "PushArraySized");
	memoryPoolCode.InitListedMemory = (memory_pool_init_listed_memory*)GetProcAddress(memoryPoolLibrary, "InitializeListedMemory");
	memoryPoolCode.AddListedItem = (memory_pool_add_listed_item*)GetProcAddress(memoryPoolLibrary, "AddListedItem");
	memoryPoolCode.RemoveListedItem = (memory_pool_remove_listed_item*)GetProcAddress(memoryPoolLibrary, "RemoveListedItem");
	memoryPoolCode.AddToEndOfList = (memory_pool_add_to_end*)GetProcAddress(memoryPoolLibrary, "AddToEndOfList");
	memoryPoolCode.RemoveSpecificNode = (memory_pool_remove_specific_node*)GetProcAddress(memoryPoolLibrary, "RemoveSpecificNode");
    }

    program_memory memory = {};

    memory.transientStorageSize = Megabytes(64);
    memory.permanentStorageSize = Gigabytes(1);

    memoryPoolCode.PoolAlloc(0, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE, &memory);

    programArenas = (win32_arenas*)memory.permanentStorage;

    size_t initArenaAllocSize = Megabytes(700);
    size_t perFrameArenaAllocSize = Megabytes(50);

    memory.permanentArenaBase = (u8*)memory.permanentStorage + sizeof(win32_arenas);
    memory.transientArenaBase = (u8*)memory.transientStorage + sizeof(win32_arenas);

    memoryPoolCode.InitArena(&programArenas->InitArena, initArenaAllocSize, &memory, e_arena_type::permanent);
    memoryPoolCode.InitArena(&programArenas->perFrameArena, perFrameArenaAllocSize, &memory, e_arena_type::transient);


    
    game_camera gCamera;
    dx_camera dxCam;


    //Win32 Framework Module
    HMODULE win32FrameworkLibrary = LoadLibrary("D:/ExternalCustomAPIs/Win32/dll/win32_framework.dll");

    if (win32FrameworkLibrary)
    {
	win32Code.Win32ProcessPendingMessages =
	    (win32_process_pending_messages*)GetProcAddress(win32FrameworkLibrary, "Win32ProcessPendingMessages");
	win32Code.Win32LoadGameCode =
	    (win32_load_game_code*)GetProcAddress(win32FrameworkLibrary, "Win32LoadGameCode");
	win32Code.Win32GameCodeSetup =
	    (win32_game_code_setup*)GetProcAddress(win32FrameworkLibrary, "Win32GameCodeSetup");
	win32Code.Win32CheckAndLoadGameCode =
	    (win32_check_and_load_game_code*)GetProcAddress(win32FrameworkLibrary, "CheckAndLoadGameCode");
	win32Code.Win32CreateSpawnableBuffers =
	    (win32_create_spawnable_buffers*)GetProcAddress(win32FrameworkLibrary, "Win32CreateSpawnableBuffers");
	win32Code.Win32FromV4ToXMVECTOR =
	    (win32_from_v4_to_xmvector*)GetProcAddress(win32FrameworkLibrary, "FromV4ToXMVECTOR");
	win32Code.Win32ConvertGameCameraToWin32 =
	    (win32_convert_game_camera_to_win32*)GetProcAddress(win32FrameworkLibrary, "ConvertGameCameraDataToWin32");
	win32Code.Win32FromM4ToXMMATRIX =
	    (win32_from_m4_to_xmmatrix*)GetProcAddress(win32FrameworkLibrary, "FromM4ToXMMATRIX");
    }

    HMODULE parseOBJLibrary = LoadLibrary("D:/ExternalCustomAPIs/OBJLoader/dll/obj_loader.dll");
    if (parseOBJLibrary)
    {
	parseObjCode.ParseOBJData = (parse_obj_data*)GetProcAddress(parseOBJLibrary, "ParseOBJData");
    }


    game_code_path_cluster gamePaths = 
	win32Code.Win32GameCodeSetup("sail_game_layer.dll",
				     "sail_temp_layer.dll",
				     "sail_lock_layer.tmp",
				     &programState);
    


    //Sail layer code
    win32_game_code sailGameCode =
	win32Code.Win32LoadGameCode(&gamePaths,
				    "SailUpdate",
				    "SailInitialize",
				    &memoryPoolCode,
				    &programArenas->InitArena);
    u32 loadCounter = 120;
    
    sail_update* SailUpdate = (sail_update*)sailGameCode.GameUpdate;
    sail_initialize* SailInitialize = (sail_initialize*)sailGameCode.GameInitialize;

    platform_info platformInfo = {};
    platformInfo.aspect = aspect;
    /*
    memory_arena* setupArena;
    memory_arena* perFrameArena;
    memory_arena* spawnedObjectArena;
      
     */


    platformInfo.frameworkArenas.setupArena = &programArenas->InitArena;
    platformInfo.frameworkArenas.perFrameArena = &programArenas->perFrameArena;

    platformInfo.parseObjCode = &parseObjCode;

    sail_initialize_data sailInitData = {};
    game_camera gameCamera = SailInitialize(&sailInitData, &gameFrameworkCode, &memoryPoolCode, &platformInfo, &memory);

    
    D3D_FEATURE_LEVEL levels[] = {
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_9_3,
	D3D_FEATURE_LEVEL_9_2,
	D3D_FEATURE_LEVEL_9_1,    
    };

    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT|D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;

    IDXGIAdapter* adapter = NULL;
    IDXGIFactory6* factory = 0;

    HR(CreateDXGIFactory1(__uuidof(IDXGIFactory6), (void**)&factory));

    factory->EnumAdapters(1, &adapter);

    IDXGIOutput* adapterOutput = {};
    adapter->EnumOutputs(1, &adapterOutput);

    HRESULT hr = {};
    
    hr = D3D11CreateDevice(adapter,
			   D3D_DRIVER_TYPE_UNKNOWN,
			   0,
			   deviceFlags,
			   levels,
			   ArrayCount(levels),
			   D3D11_SDK_VERSION,
			   &d3dDevice,
			   &featureLevel,
			   &context);

    WNDCLASS wc = {};
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = Win32MainWindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = "Sailing";


    win32_spawnable_objs win32Buffers = {};
    win32Code.Win32CreateSpawnableBuffers(&sailInitData.gameObjs,
					  &win32Buffers,
					  platformInfo.frameworkArenas.spawnedObjectArena,
					  &programArenas->perFrameArena,
					  &memoryPoolCode,
					  d3dDevice);    
    if (RegisterClass(&wc))
    {
	RECT rect = {};
	SetRect(&rect, 0, 0, (i32)screenWidth, (i32)screenHeight);

	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

	i32 windowWidth = rect.right - rect.left;
	i32 windowHeight = rect.bottom - rect.top;

	HWND window = CreateWindow(
	    wc.lpszClassName,
	    "Sailing",
	    WS_OVERLAPPEDWINDOW|WS_VISIBLE,
	    CW_USEDEFAULT, CW_USEDEFAULT,
	    windowWidth, windowHeight,
	    0, 0,
	    hInstance,
	    0);

	if (window)
	{
	    //Get our refresh rate
	    i32 monitorRefreshRate = 60;


#if 0	    
	    HDC refreshDC = GetDC(window);
	    i32 win32RefreshRate = GetDeviceCaps(refreshDC, VREFRESH);
	    if (win32RefreshRate > 1)
	    {
		monitorRefreshRate = win32RefreshRate;
	    }
#endif

	    r32 gameUpdateHz = (monitorRefreshRate / 2.0f);
	    r32 targetSecondsPerFrame = 1.0f / (r32)monitorRefreshRate;
	    
	    //Creating our swap chain
	    DXGI_SWAP_CHAIN_DESC1 desc = {};
	    desc.BufferCount = 2;
	    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	    desc.SampleDesc.Count = 1;
	    desc.SampleDesc.Quality = 0;
	    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	    desc.Width = windowWidth;
	    desc.Height = windowHeight;
	    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	    hr = factory->CreateSwapChainForHwnd(
		d3dDevice,
		window,
		&desc,
		0,
		0,
		&swapChain);

	    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	    hr = d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);

	    backBuffer->GetDesc(&bbDesc);

	    CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		(UINT)bbDesc.Width,
		(UINT)bbDesc.Height,
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL);
	    hr = d3dDevice->CreateTexture2D(&depthStencilDesc,
					    nullptr,
					    &depthStencil);

	    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);

	    hr = d3dDevice->CreateDepthStencilView(depthStencil,
						   &depthStencilViewDesc,
						   &depthStencilView);

	    D3D11_VIEWPORT viewport = {};
	    viewport.Height = (r32)bbDesc.Height;
	    viewport.Width = (r32)bbDesc.Width;
	    viewport.MinDepth = 0;
	    viewport.MaxDepth = 1;

	    context->RSSetViewports(1, &viewport);

	    shaders gameShaders = {};
	    
	    CreateShaders(&gameShaders);

	    game_input input[2] = {};
	    game_input* newInput = &input[0];
	    game_input* oldInput = &input[1];
	    
	    programState.running = true;

	    LARGE_INTEGER frequency = {};
	    QueryPerformanceFrequency(&frequency);
	    perfCountFrequency = frequency.QuadPart;
	    
	    LARGE_INTEGER startTime;
	    LARGE_INTEGER endTime;

	    LARGE_INTEGER lastCounter;
	    QueryPerformanceCounter(&lastCounter);
	    
	    r32 deltaTime = 0.0f;

	    //Init our dynamic constant buffer
	    //This can be moved outside to a new function when
	    //need/when I create more buffers for the game, but rn it's just here
	    D3D11_BUFFER_DESC cbDesc = {};
	    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	    cbDesc.ByteWidth = sizeof(object_constants);
	    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	    
	    sail_constant_buffers sailConstantBuffers = {};
	    
	    hr = d3dDevice->CreateBuffer(&cbDesc, NULL, &sailConstantBuffers.dynamicVBuffer);


	    RAWINPUTDEVICE rid[1];
	    rid[0].usUsagePage = 0x01;
	    rid[0].usUsage = 0x02;
	    rid[0].dwFlags = RIDEV_NOLEGACY;
	    rid[0].hwndTarget = window;

	    if (RegisterRawInputDevices(rid, 1, sizeof(rid[0])) == FALSE)
	    {
		OutputDebugString("Input device NOT registered\n");
	    }
	    while (programState.running)
	    {
		QueryPerformanceCounter(&startTime);
		
		loadCounter = win32Code.Win32CheckAndLoadGameCode(&gamePaths,
								  &sailGameCode,
								  &memoryPoolCode,
								  &programArenas->InitArena);

		game_controller_input* oldKeyboardController = GetController(oldInput, 0);
		game_controller_input* newKeyboardController = GetController(newInput, 0);
		
		newKeyboardController->isConnected = true;
		for (i32 buttonIndex = 0; buttonIndex < ArrayCount(newKeyboardController->buttons); ++buttonIndex)
		{
		    newKeyboardController->buttons[buttonIndex].endedDown =
			oldKeyboardController->buttons[buttonIndex].endedDown;
		}
		mouse_movements mouse = {};
		
		win32Code.Win32ProcessPendingMessages(newKeyboardController,
						      oldKeyboardController,
						      newInput, oldInput,
						      &mouse,
						      &memoryPoolCode,
						      &programArenas->perFrameArena,
						      &programState);

		gameCamera.xChange = deltaTime * (0.3f * mouse.loc.x);
		gameCamera.yChange = deltaTime * (0.3f * mouse.loc.y);
		//The rest of the stuff (besides rendering) we see in win32_dx11.cpp should all be moved to our
		//game code bc it's something we want to separate from the platform, and since
		//I spent the time creating a separate math library, I would actually like to use it

		SailUpdate(&gameFrameworkCode, &memoryPoolCode, newInput, &gameCamera, deltaTime);


		game_camera_data gCamData = {};
		gCamData.world = gameCamera.world;
		gCamData.view = gameCamera.view;
		gCamData.projection = gameCamera.projection;
		
		win32Code.Win32ConvertGameCameraToWin32(&dxCam, &gCamData);

		Render(&sailInitData.gameObjs, &win32Buffers, &sailConstantBuffers, &gameShaders, &dxCam);

		swapChain->Present(1, 0);

		LARGE_INTEGER workCounter = {};
		QueryPerformanceCounter(&workCounter);
		r32 workSecondsElapsed = Sail32GetSecondsElapsed(lastCounter, workCounter);
		r32 secondsElapsedForFrame = workSecondsElapsed;

		if (secondsElapsedForFrame < targetSecondsPerFrame)
		{
		    DWORD sleepMs = 0;
		    while (secondsElapsedForFrame < targetSecondsPerFrame)
		    {
			if (sleepIsGranular)
			{
			    sleepMs = (DWORD)(1000.0f * (targetSecondsPerFrame - secondsElapsedForFrame));
			    if (sleepMs > 0)
			    {
				Sleep(sleepMs - 1);
			    }
			}
			secondsElapsedForFrame = Sail32GetSecondsElapsed(lastCounter,
									 Sail32GetWallClock());
			
		    }
		    r32 testSecondsForFrame = Sail32GetSecondsElapsed(lastCounter, Sail32GetWallClock());
		    if (testSecondsForFrame < targetSecondsPerFrame)
		    {
			//Frame was missed error
		    }
		}
		else
		{
		    //also missed a frame
		}

		//spin

		

		//after spin
		LARGE_INTEGER endCounter = Sail32GetWallClock();

		r32 msPerFrame = (1000.0f * Sail32GetSecondsElapsed(lastCounter,
								    Sail32GetWallClock()));

		lastCounter = endCounter;
		game_input* temp = newInput;
		newInput = oldInput;
		oldInput = temp;

		memoryPoolCode.ClearArena(&programArenas->perFrameArena);

		QueryPerformanceCounter(&endTime);
		LONGLONG elapsed = endTime.QuadPart - startTime.QuadPart;
		deltaTime = (r32)((r32)elapsed / (r32)frequency.QuadPart);


	    }
	}
    }
	
    if (!programState.running)
    {
	i32 foo = 0;
    }
    return(0);
}
