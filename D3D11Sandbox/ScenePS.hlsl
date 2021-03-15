struct PS_INPUT
{
	float4 Position : SV_Position;
	float4 Color	: COLOR0;
};

float4 main(PS_INPUT input) : SV_Target
{
	float4 color = input.Color;
	return color;
}