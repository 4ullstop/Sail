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

	float fogHeightFalloff;
	float fogBaseHeight;
	float baseFogDensity;

}

cbuffer transforms : register(b1)
{
	float4x4 invViewProj;
	float4 cameraWorldPos;
	float4 sunDirection;
};

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

float3 GetWorldPosition(float2 uv)
{
	float depth = gDepthTexture.Sample(gLinearSampler, uv).r;

	//uv&depth to NDC
	float x = uv.x * 2.0f - 1.0f;
	float y = (1.0f - uv.y) * 2.0f - 1.0f;
	float z = depth;

	float4 ndcPos = float4(x, y, z, 1.0f);
	float4 worldPos = mul(ndcPos, invViewProj);

	return(worldPos.xyz / worldPos.w);
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float4 sceneColor = gSceneTexture.Sample(gLinearSampler, input.texCoord);
	float depth = gDepthTexture.Sample(gLinearSampler, input.texCoord).r;

	if (depth >= 1.0f)
	{
		return(sceneColor);
	}
	float distance = GetLinearDepth(input.texCoord);
	float3 pixelWorldPos = GetWorldPosition(input.texCoord);


	float cameraHeight = cameraWorldPos.y;
	float pixelHeight = pixelWorldPos.y;

	float hAvg = (cameraHeight + pixelHeight) * 0.5f;
	float heightFalloff = exp(-fogHeightFalloff * (hAvg - fogBaseHeight));

	float fogExp = distance * baseFogDensity * heightFalloff;
	float fogFactor = exp(-fogExp);

	fogFactor = saturate(fogFactor);
	float3 finalColor = lerp(fogColor.rgb, sceneColor.rgb, fogFactor);
	return (float4(finalColor, sceneColor.a));
}