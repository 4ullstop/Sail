Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

struct VS_INPUT
{
	float2 pos : POSITION;
	float2 uv : TEXCOORD;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer UITransform : register(b0)
{
	float2 pos;
	float2 uv;
}

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;
	output.pos = float4(input.pos, 0.0f, 1.0f);
	output.uv = input.uv;
	return(output);
}

float4 PS(PS_INPUT input) : SV_TARGET
{
	return gTexture.Sample(gSampler, input.uv);
}