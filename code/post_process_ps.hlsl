Texture2D gSceneTexture : register(t0);
Texture2D<float> gDepthTexture : register(t1);
SamplerState gLinearSampler : register(s0);

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

cbuffer fog : register(b0)
{
	float4 fogColor;
	float gFogStart;
	float gFogEnd;
	float nearZ;
	float farZ;
}

//Convert our non-linear depth buffer value back to physical world distance
float GetLinearDepth(float2 uv)
{
	float depth = gDepthTexture.Sample(gLinearSampler, uv).r;

	if (depth >= 1.0f)
	{
		return(farZ);
	}
	else
	{
		return((nearZ * farZ) / (farZ - depth * (farZ + nearZ)));
	}
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float4 sceneColor = gSceneTexture.Sample(gLinearSampler, input.texCoord);
	float distance = GetLinearDepth(input.texCoord);

	float fogDensity = 0.01f;
	float fogFactor = exp(-distance * fogDensity);
	fogFactor = saturate(fogFactor);
	float3 finalColor = lerp(fogColor.rgb, sceneColor.rgb, fogFactor);
	return (float4(finalColor, sceneColor.a));
}