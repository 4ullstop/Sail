struct VS_INPUT
{
	float3 vPos : POSITION;
	float3 vColor : COLOR0;
	float2 texCoord : TEXCOORD;
	uint id : SV_VertexID;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float2 texCoord : TEXCOORD;
	float4 normal : NORMAL0;
	float4 viewForward : NORMAL1;
};

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix mWorld;
	matrix view;
	matrix projection;
}

cbuffer UpdateBuffer : register(b1)
{
	float4x4 modelWorld;
	float t;
}

cbuffer ConstantSineWaves : register(b2)
{
	float4 amp;
	float4 frequency;
	float4 phase;
}

static const float2 waveDirections[4] =
{
	float2(1.0f, 0.0f),
	float2(0.707f, 0.707f),
	float2(0.5f, 1.0f),
	float2(-0.6f, 0.8f)		     
};
	
VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	float4 pos = float4(input.vPos, 1.0f);

	float totalY = 0.0f;
	float dYdx = 0.0f;
	float dYdz = 0.0f;

	for (int i = 0; i < 4; i++)
	{
		float2 d = waveDirections[i];
		float dotPos = dot(d, pos.xz);
		float phaseAngle = dotPos * frequency[i] + t * phase[i];

		totalY += amp[i] * sin(phaseAngle);

		float cosVal = cos(phaseAngle);
		dYdx += amp[i] * frequency[i] * d.x * cosVal;
		dYdz += amp[i] * frequency[i] * d.y * cosVal;
	}

	pos.y += totalY;

	float3 binormal = float3(1.0f, dYdx, 0.0f);
	float3 tangent = float3(0.0f, dYdz, 1.0f);

	float3 objectNormal = normalize(cross(tangent, binormal));

	float4 worldPos = mul(pos, modelWorld);
	float4 viewPos = mul(worldPos, view);
	float4 projPos = mul(viewPos, projection);

	float3 worldNormal = normalize(mul(float4(objectNormal, 0.0f), modelWorld).xyz);

	output.position = projPos;
	output.color = float4(input.vColor, 1.0f);
	output.texCoord = input.texCoord;
	output.normal = float4(worldNormal, 0.0f);
	output.viewForward = float4(view[3][0], view[3][1], view[3][2], 0.0f);

	return(output);
}