struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

VS_OUTPUT VS(uint VertexID : SV_VertexID)
{
	VS_OUTPUT output;
	output.texCoord = float2((VertexID << 1) & 2, VertexID & 2);
	output.position = float4(output.texCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return(output);
}