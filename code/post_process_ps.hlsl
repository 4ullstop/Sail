Texture2D gSceneTexture : register(t0);
SamplerState gLinearSampler : register(s0);

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float4 color = gSceneTexture.Sample(gLinearSampler, input.texCoord);
	float luminance = dot(color.rgb, float3(0.2126f, 0.7152f, 0.0722f));
	return float4(luminance, luminance, luminance, color.a);
}