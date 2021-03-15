
cbuffer WVPConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
};

struct VS_INPUT 
{
	float3 vPos : POSITION;
	float3 vColor : COLOR0;
	float3 vNormal : NORMAL;
};

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float4 Color	: COLOR0;
};

VS_OUTPUT main( VS_INPUT input )
{
	VS_OUTPUT output;

	float4 pos = float4(input.vPos, 1.0f);
	pos = mul(pos, World);
	pos = mul(pos, View);
	pos = mul(pos, Projection);

	output.Position = pos;
	output.Color = float4(input.vColor, 1.0f);

	return output;
}