struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
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
	float3 sunColor;
	float lightIntensity;
};

float3 CalculateFaceNormal(float3 worldPosition)
{
	float3 dx = ddx(worldPosition);
	float3 dy = ddy(worldPosition);

	float3 faceNormal = cross(dx, dy);
	return(normalize(faceNormal));
}

PS_OUTPUT main(PS_INPUT In)
{
	PS_OUTPUT output;
	float4 texColor = objTexture.Sample(objSampler, In.tex);

	float3 N = CalculateFaceNormal(In.position.xyz);
	float3 L = normalize(-sunNormal.xyz);
	float3 nDotL = saturate(dot(N, L));
	float4 baseColor = In.color;
	if (hasMaterials.x == 1.0f)
	{
		baseColor = MaterialPropertiesBuffer[FaceToMaterialMap[In.primID]].matColor;
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
	
	float3 diffuse = baseColor.rgb * nDotL * (sunColor * lightIntensity);

	float3 ambient = baseColor.rgb * 0.1f;

//	output.RGBColor = baseColor;
	output.RGBColor = float4(diffuse + ambient, 1.0f);
	return(output);
}