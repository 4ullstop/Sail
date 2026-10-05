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
};

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix mWorld;
	matrix view;
	matrix projection;
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

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			float2 d = float2(directions[i][j] + dYdx, directions[j][i] + dYdz);
			d = normalize(d);
			float e = 2.71828182;
			float dotPos = dot(d, pos.xz);
			float phaseAngle = dotPos * frequency[i][j] + t * phase[i][j];

//			totalY += pow(2.71828182, ((amp[i][j] * sin(phaseAngle)) - 1.5));

			totalY += amp[i][j] * pow(e, sin(phaseAngle) - 1);

			float cosVal = cos(phaseAngle);
#if 0
			dYdx += amp[i][j] * frequency[i][j] * d.x * cosVal;
			dYdz += amp[i][j] * frequency[i][j] * d.y * cosVal;
#else
			dYdx += frequency[i][j] * d.x * pow(e, ((amp[i][j] * cosVal) - 1.0f));
			dYdz += frequency[i][j] * d.y * pow(e, ((amp[i][j] * cosVal) - 1.0f));		
#endif
		}
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
	output.cameraPosition = cameraPosition;

	return(output);
}