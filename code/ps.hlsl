struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 cameraPosition : POSITION0;
	float4 color : COLOR0;
	float3 worldPos : TEXCOORD1;
	float2 tex : TEXCOORD0;
	uint primID: SV_PrimitiveID;
};

struct PS_OUTPUT
{
	float4 RGBColor : SV_TARGET;
};

struct material_properties
{
	float4 matColor;
	float4 specular; //specular exponent is w value
	float4 ambient;
	float4 emmisive;
};

StructuredBuffer<uint> FaceToMaterialMap : register(t0);

StructuredBuffer<material_properties> MaterialPropertiesBuffer: register(t1);

Texture2D objTexture : register(t2);
SamplerState objSampler : register(s0);

cbuffer ObjectPropertiesBuffer : register(b0)
{
	float4 hasMaterials;	
};

cbuffer GlobalSun : register(b1)
{
	float4 sunNormal;
	float4 sunColor;
};

float3 CalculateFaceNormal(float3 worldPosition)
{
	float3 dx = ddx(worldPosition);
	float3 dy = ddy(worldPosition);

	float3 faceNormal = cross(dx, dy);


	return (normalize(faceNormal));
}

PS_OUTPUT main(PS_INPUT In)
{
	PS_OUTPUT output;
	float4 texColor = objTexture.Sample(objSampler, In.tex);

	float3 N = CalculateFaceNormal(In.worldPos);
	float3 L = normalize(sunNormal.xyz);
	float3 V = normalize(In.cameraPosition.xyz - In.position.xyz);
	float3 nDotL = saturate(dot(N, L));
	float4 baseColor = In.color;
	float3 specular = float3(0.0f, 0.0f, 0.0f);
	float specularExponent = 0.0f;

	float fresnelStrength = 0.1f;
	float fresnelShininess = 0.5f;
	float3 fresnelColor = float3(0.2f, 0.2f, 0.2f);
	float fresnelBias = 1.0f;
	float3 fresnelNormal = N;
	fresnelNormal.xz *= fresnelStrength;
	fresnelNormal = normalize(fresnelNormal);
	float base = 1 - dot(V, fresnelNormal);
	float exponential = pow(base, fresnelShininess);
	float R = exponential + fresnelBias * (1.0f - exponential);
	R *= fresnelStrength;
	float3 fresnel = fresnelColor * R;

	if (hasMaterials.x == 1.0f)
	{
		baseColor = MaterialPropertiesBuffer[FaceToMaterialMap[In.primID]].matColor;
		specular = MaterialPropertiesBuffer[FaceToMaterialMap[In.primID]].specular.xyz;
		specularExponent = MaterialPropertiesBuffer[FaceToMaterialMap[In.primID]].specular.w;
		float3 H = normalize(V + L);
		float nDotH = saturate(dot(N, H));
		specular *= pow(nDotH, specularExponent);
		specular *= (nDotL > 0.0f ? 1.0f : 0.0f);
		specular *= fresnel;
	}

	if (hasMaterials.y == 1.0f)
	{
		baseColor = texColor;
	}

	if (hasMaterials.z == 1.0f)
	{
		output.RGBColor = baseColor;
		return(output);
	}
	float3 newSun = float3(0.8f, 0.8f, 0.8f);
	float3 diffuse = baseColor.rgb * nDotL * (sunColor.xyz * sunColor.w);

	float3 ambient = baseColor.rgb * 0.3f;

//	output.RGBColor = baseColor;
	output.RGBColor = float4(diffuse + ambient + specular + fresnel, 1.0f);
//	output.RGBColor = float4(N * 0.5f + 0.5f, 1.0f);
	return(output);
}