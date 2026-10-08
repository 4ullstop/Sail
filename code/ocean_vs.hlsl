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
	float4 cameraPosition : NORMAL1;
	float4 cameraForward : FORWARD0;
};

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix mWorld;
	matrix view;
	matrix projection;
	matrix viewInverted;
	float4 cameraPosition;
}

cbuffer UpdateBuffer : register(b1)
{
	float4x4 modelWorld;
	float t;
}

cbuffer ConstantSineWaves : register(b2)
{
	float4x4 amp;
	float4x4 frequency;
	float4x4 phase;
	float4x4 directions;
}
	
VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	float4 pos = float4(input.vPos, 1.0f);

	float totalY = 0.0f;
	float dYdx = 0.0f;
	float dYdz = 0.0f;

	float e = 2.71828182f;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			float2 d = normalize(float2(directions[i][j], directions[j][i]));

			float dotPos = dot(d, pos.xz);
			float phaseAngle = dotPos * frequency[i][j] + t * phase[i][j];

			float sinVal = sin(phaseAngle);
			float cosVal = cos(phaseAngle);

			float expTerm = pow(e, sinVal - 1.0f);
			totalY += amp[i][j] * expTerm;

			float dTerm = amp[i][j] * expTerm * cosVal * frequency[i][j];

			dYdx += dTerm * d.x;
			dYdz += dTerm * d.y;
		}
	}

	pos.y += totalY;

	float3 objectNormal = normalize(float3(-dYdx, 1.0f, -dYdz));

	float3 worldNormal = normalize(mul(objectNormal, (float3x3)modelWorld));

	float4 worldPos = mul(pos, modelWorld);
	float4 viewPos = mul(worldPos, view);
	float4 projPos = mul(viewPos, projection);


	output.position = projPos;
	output.color = float4(input.vColor, 1.0f);
	output.texCoord = input.texCoord;
	output.normal = float4(worldNormal, 0.0f);
	output.cameraPosition = cameraPosition;
	output.cameraForward = normalize(float4(viewInverted[2][0], viewInverted[2][1], viewInverted[2][2], viewInverted[2][3]));
	return(output);
}