struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
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

cbuffer ObjectPropertiesBuffer : register(b0)
{
	float4 hasMaterials;	
};

PS_OUTPUT main(PS_INPUT In)
{
	PS_OUTPUT output;
	if (hasMaterials.x == 1.0f)
	{
		output.RGBColor = MaterialPropertiesBuffer[FaceToMaterialMap[In.primID]].matColor;
	}
	else
	{
	   	output.RGBColor = In.color;
	}
	return(output);
}