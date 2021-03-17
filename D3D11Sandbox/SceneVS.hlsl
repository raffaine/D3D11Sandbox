
cbuffer WVPConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
};

cbuffer LightSettings : register(b1)
{
	float4 LightPos;
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
	matrix WV = mul(World, View);
	matrix WVP = mul(WV, Projection);

	// I'm just factoring the light angle based on its position
	// and the object, it's position is irrelevant for atenuation
	// so it doesn't matter how far the light is ... which has
	// below average results ... but this is a very simple shader
	// just so I can see stuff ... anyway, attenuation is a TODO
	float3 light = normalize(mul(LightPos, View).xyz);
	float3 normal = normalize(mul(float4(input.vNormal, 1.0f), WV).xyz);
	output.Position = mul(pos, WVP);
	output.Color = float4(input.vColor * max(dot(normal, light), 0.0f), 1.0f);

	return output;
}