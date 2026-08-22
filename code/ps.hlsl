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

PS_OUTPUT main(PS_INPUT In)
{
	PS_OUTPUT output;
	float4 texColor = objTexture.Sample(objSampler, In.tex);

	float4 baseColor = In.color;
	if (hasMaterials.x == 1.0f)
	{
		baseColor = MaterialPropertiesBuffer[FaceToMaterialMap[In.primID]].matColor;
	}

	if (hasMaterials.y == 1.0f)
	{
		output.RGBColor = texColor;
	}	
	else
	{
		output.RGBColor = baseColor;
	}
	return(output);
}