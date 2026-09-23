Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

struct PS_OUTPUT
{
	float4 sample;		
};


PS_OUTPUT main(PS_INPUT input) : SV_TARGET
{
	PS_OUTPUT result;
	result.sample = gTexture.Sample(gSampler, input.uv);
	return(result);
}