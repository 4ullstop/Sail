cbuffer LineSegment : register(b0)
{
	float2 a;
	float2 b;
	float2 q;
	float4 lineColor;
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

float LineSegment2DSDF(float2 inP, float2 inA, float2 inB)
{
	float2 ba = inB - inA;
	float2 pa = inP - inA;
	float k = saturate(dot(pa, ba) / dot(ba, ba));
	return (length(pa - ba * k));
}


float4 PS(PS_INPUT input) : SV_TARGET
{
	float dist = LineSegment2DSDF(q, a, b);
	float halfThickness = lineColor.a * 0.5f;//stored thickness in alpha section of color

	float alpha = 1.0f - smoothstep(halfThickness - 0.002, halfThickness + 0.002, dist);
	float4 result = float4(lineColor.rgb, lineColor.a * alpha);
	return(result);
}