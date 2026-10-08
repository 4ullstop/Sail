TextureCube cubeMap : register(t0);
SamplerState cubeMapSampler : register(s0);
	
struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float2 tex : TEXCOORD0;
	float4 normal : NORMAL0;
	float4 cameraPosition : NORMAL1;
	float4 cameraForward : FORWARD;
};

struct PS_OUTPUT
{
	float4 RGBColor : SV_TARGET;	
};

cbuffer GlobalSun : register(b0)
{
	float4 sunNormal;
	float4 sunLocation;
}

cbuffer Transforms : register(b1)
{
	float4x4 invView;
}

struct directional_output
{
	float3 diffuse;
	float3 specular;
};

directional_output ComputeDirectionalLight(float3 toEye, float3 inRGB, float3 lightSpec, float4 surfaceNormal)
{
	directional_output result = {float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f)};
	float4 lightVec = -sunNormal;
	float diffuseFactor = dot(lightVec, surfaceNormal);

	if (diffuseFactor > 0.0f)
	{
		float4 v = reflect(-lightVec, surfaceNormal);
		float specExp = 10.0f;
		float specFactor = pow(max(dot(v.xyz, toEye), 0.0f), 10.0f);
		result.diffuse = diffuseFactor * inRGB * lightSpec;
		result.specular = specFactor * specExp * lightSpec;
	}
	return(result);
}

PS_OUTPUT main(PS_INPUT In)
{
	PS_OUTPUT output;



	float3 N = normalize(In.normal.xyz);
	float3 L = normalize(-sunNormal.xyz);

	float nDotL = saturate(dot(N, L));
	float3 sunColor = float3(0.1f, 0.1f, 0.1f);
	float3 diffuse = In.color.rgb * nDotL * sunColor;
	float3 V = normalize(In.cameraPosition.xyz - In.position.xyz);




	float3 H = normalize(V + L);
	float nDotH = saturate(dot(N, H));
	
	float specExp = 10.0f;
	float3 sunSpecColor = float3(0.8f, 0.8f, 0.8f);
	float3 specular = pow(nDotH, specExp) * sunSpecColor;
	specular *= (nDotL > 0.0f ? 1.0f : 0.0f);




	float fresnelStrength = 0.2f;
	float fresnelShininess = 0.2f;
	float3 fresnelColor = float3(0.8f, 0.8f, 0.9f);
	float fresnelBias = 5.0f;

	float3 fresnelNormal = N;
	fresnelNormal.xz *= fresnelStrength;
	fresnelNormal = normalize(fresnelNormal);
	float base = 1 - dot (V, fresnelNormal);

	float exponential = pow(base, fresnelShininess);
	float R = exponential + fresnelBias * (1.0f - exponential);
	R *= fresnelStrength;
	float3 fresnel =  fresnelColor * R;
	
	float4 reflectedDir = float4(reflect(-V, N), 1.0f);
	float3 cube = cubeMap.Sample(cubeMapSampler, reflectedDir.xyz).rgb;

	fresnel += cube.rgb * R;
	specular *= fresnel;

	float3 ambient = In.color.rgb * 0.1f;
	output.RGBColor = float4((ambient + diffuse + specular + fresnel), In.color.a);


	return(output);
}