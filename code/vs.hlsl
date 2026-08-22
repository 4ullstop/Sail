struct VS_INPUT
{
	float3 vPos : POSITION;
	float3 vColor : COLOR0;
	float2 texCoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float2 texCoord : TEXCOORD;
};

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix mWorld;
	matrix view;
	matrix projection;
}

cbuffer ModelWorldBuffer : register(b1)
{
	float4x4 modelWorld;
}

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	float4 pos = float4(input.vPos, 1.0f);

//	pos = mul(pos, mWorld);
	
	pos = mul(pos, modelWorld);
	pos = mul(pos, view);
	pos = mul(pos, projection);

	output.position = pos;
	output.color = float4(input.vColor, 1.0f);
	output.texCoord = input.texCoord;

	return(output);
}