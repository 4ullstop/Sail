#include "ocean.h"


internal void
CreateOceanGridWaveValues(game_ocean* ocean)
{
    r32 amp = 1.0f;
    r32 waveLen = 1.0f;
    for (i32 i = 0; i < MAX_WAVE_COUNT; i++)
    {
	r32 ampDec = 0.7f;
	r32 waveLenInc = 1.18f;
	amp *= ampDec;
	ocean->amplitudes[i] = amp;
	waveLen *= waveLenInc;	
	ocean->frequencies[i] = 2 / waveLen;

	r32 randSpeed = RandomFloatInRange(0.5f, 1.5f);
	ocean->phases[i] = (randSpeed * (2 / waveLen));
	r32 randDirection = RandomFloatInRange(0.0f, 1.0f);
	ocean->directions[i] = randDirection;
    }
}

internal game_ocean
InitGameOceanValues()
{
    game_ocean result = {};
    v4 oceanLoc = {0.0f, BOATFROMWATER, 0.0f, 0.0f};
    v4 oceanRot = QuaternionIdentity();
    v4 oceanScale = {1.0f, 1.0f, 1.0f, 1.0f};
    
    result.oceanModelMat = CreateModelMatrix(oceanScale,
						oceanRot,
						oceanLoc);
    result.oceanModelMat = result.oceanModelMat * Identity();
    result.sliceWidth = 1200;
    result.sliceLength = 1200;
    result.width = 400.0f;
    result.length = 400.0f;


    result.vertexCountX = result.sliceWidth + 1;
    result.vertexCountZ = result.sliceLength + 1;
    result.resolution = result.vertexCountX * result.vertexCountZ;
    result.planeSize = 0.5f;

    CreateOceanGridWaveValues(&result);
    
    return(result);
}

internal r32
CalculatePositionFromGrid(v2 pos, game_ocean* ocean)
{
    r32 totalY = 0.0f;
    r32 e = 2.71828182f;
    r32 dYdx = 0.0f;
    r32 dYdz = 0.0f;


    i32 width = 4;

    for (i32 i = 0; i < MAX_WAVE_COUNT; i++)
    {
	i32 row = i / width;
	i32 col = i % width;
	i32 endIndex = (row * 4) + col;
	v2 directions = {ocean->directions[i], ocean->directions[endIndex]};
	v2 d = Normalize(directions);

	r32 dotPos = Dot(d, pos);
	r32 phaseAngle = dotPos * ocean->frequencies[i] + ocean->time * ocean->phases[i];

	r32 sinVal = sinf(phaseAngle);
	r32 cosVal = cosf(phaseAngle);

	r32 expTerm = (r32)pow(e, sinVal - 1.0f);
	totalY += ocean->amplitudes[i] * expTerm;
	r32 dTerm = ocean->amplitudes[i] * expTerm * cosVal * ocean->frequencies[i];

	dYdx += dTerm * d.x;
	dYdz += dTerm * d.y;
    }
    
    return(totalY);
}
