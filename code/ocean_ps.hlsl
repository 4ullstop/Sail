struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float2 tex : TEXCOORD0;
	float4 normal : NORMAL0;
	float4 viewForward : NORMAL1;
};

struct PS_OUTPUT
{
	float4 RGBColor : SV_TARGET;	
};

cbuffer GlobalSun : register(b0)
{
	float4 sunNormal;
	float4 sunLocation;
}

PS_OUTPUT main(PS_INPUT In)
{
	PS_OUTPUT output;

	float3 N = normalize(In.normal.xyz);
	float3 L = normalize(sunNormal.xyz);

	float nDotL = saturate(dot(N, L));

	float3 V = normalize(In.viewForward.xyz);
	float3 H = normalize(V + L);
	float specExp = 0.5f;
	float specular = pow(saturate(dot(H, N)), specExp);

	output.RGBColor = float4(In.color.rgb * specular * nDotL, In.color.a);
	return(output);
}