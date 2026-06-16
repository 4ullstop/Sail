@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

set commonCompilerFlags=-nologo -Gm- -GR- -EHa- -Oi -WX -W4 -wd4201 -wd4100 -wd4101 -wd4189 -wd4505 -DTEST_INTERNAL=1 -DTEST_SLOW=1 -DTEST_WIN32=1 -DUSE_FORTY_MATH_FAST=1 -FC -Zi

set commonLinkerFlags=-incremental:no user32.lib gdi32.lib winmm.lib d3d11.lib dxgi.lib D3DCompiler.lib /LIBPATH:"D:\ExternalCustomAPIs\MemoryPools\dll" memory_pools.lib

REM 32-bit build
REM cl %commonCompilerFlags% ..\code\win32_handmade.cpp %commonLinkerFlags%

REM fxc compiler for shaders...
REM start "C:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64\fxc.exe" "L:\code\vshader.hlsl"

fxc.exe -nologo /Od /Zi /T vs_5_0 /Fo vs.cso "S:\code\vs.hlsl"
fxc.exe -nologo /Od /Zi /T ps_5_0 /Fo ps.cso "S:\code\ps.hlsl"


cl %commonCompilerFlags% ..\code\sail_game_layer.cpp /LD /link /EXPORT:SailUpdate /EXPORT:SailInitialize

cl %commonCompilerFlags% ..\code\sail_win32.cpp  /link %commonLinkerFlags%
popd
