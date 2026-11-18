#if !defined(OCEAN_H)

#define MAX_WAVE_COUNT 16

struct game_ocean
{
    r32 planeSize;
    r32 width;
    r32 length;
    u32 sliceWidth;
    u32 sliceLength;
    i32 resolution;
    u32 indexCount;
    m4 oceanModelMat;
    r32 time;
    u32 vertexCountX;
    u32 vertexCountZ;

    //wave init stuff
    r32 amplitudes[MAX_WAVE_COUNT];
    r32 frequencies[MAX_WAVE_COUNT];
    r32 phases[MAX_WAVE_COUNT];
    r32 directions[MAX_WAVE_COUNT];
};


#define OCEAN_H
#endif
