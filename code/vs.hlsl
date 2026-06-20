struct VS_INPUT
{
	float3 vPos : POSITION;
	float3 vColor : COLOR0;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix mWorld;
	matrix view;
	matrix projection;
}

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	float4 pos = float4(input.vPos, 1.0f);

//	pos = mul(pos, mWorld);
	pos = mul(pos, view);
	pos = mul(pos, projection);

	output.position = pos;
	output.color = float4(input.vColor, 1.0f);

	return(output);
}