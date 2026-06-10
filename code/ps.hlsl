struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

struct PS_OUTPUT
{
	float4 RGBColor : SV_TARGET;
};

PS_OUTPUT main(PS_INPUT In)
{
	PS_OUTPUT output;
	output.RGBColor = In.color;
	return(output);
}