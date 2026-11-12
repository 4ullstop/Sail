cbuffer LineSegment : register(b0)
{
	float2 a;
	float2 b;
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
	float k = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
	return (length(pa - ba * k));
}


float4 PS(PS_INPUT input) : SV_TARGET
{
	float2 pixelPos = input.position.xy;
	float dist = LineSegment2DSDF(pixelPos, a, b);
	float4 colorThick = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float halfThickness = lineColor.a * 0.5f;//stored thickness in alpha section of color

	float alpha = 1.0f - smoothstep(halfThickness - 0.002, halfThickness + 0.002, dist);
	if (alpha <= 0.0f) discard;

	float4 result = float4(lineColor.rgb, alpha);
	return(result);
}