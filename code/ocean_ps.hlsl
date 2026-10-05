struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float2 tex : TEXCOORD0;
	float4 normal : NORMAL0;
	float4 cameraPosition : NORMAL1;
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
	float3 L = normalize(-sunNormal.xyz);

	float nDotL = saturate(dot(N, L));
	float3 sunColor = float3(0.8f, 0.8f, 0.8f);
	float3 diffuse = In.color.rgb * nDotL * sunColor;
	float3 V = normalize(In.cameraPosition.xyz - In.position.xyz);	

#if 1

	float3 H = normalize(V + L);
	float nDotH = saturate(dot(N, H));
	
	float specExp = 5.0f;
	float3 sunSpecColor = float3(0.8f, 0.8f, 0.8f);
	float3 specular = pow(nDotH, specExp) * sunSpecColor;
	specular *= (nDotL > 0.0f ? 1.0f : 0.0f);

//	output.RGBColor = float4(In.color.rgb * nDotL + specular, In.color.a);
	float3 ambient = In.color.rgb * 0.1f;
	output.RGBColor = float4(ambient + diffuse + specular, In.color.a);

#else
	float3 H = normalize(V + L);
	float specExp = 10.0f;
	float specular = pow(saturate(dot(N, H)), specExp);
	output.RGBColor = float4(In.color.rgb * nDotL + specular, In.color.a);
#endif
	return(output);
}