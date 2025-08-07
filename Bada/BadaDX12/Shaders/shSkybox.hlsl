TextureCube texCubemap : register(t0);
SamplerState samplerLinearWrap : register(s0);

cbuffer CONSTANT_BUFFER_DEFAULT : register(b0)
{
    matrix g_matWorld;
    matrix g_matView;
    matrix g_matProj;
};

struct VSInput
{
    float4 pos : POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 posModel : POSITION;
};

PSInput VSMain(VSInput input)
{
    PSInput output = (PSInput) 0;
    
    output.posModel = input.pos.xyz;
    output.position = mul(float4(input.pos.xyz, 0.0), g_matView); // only Rotation
    output.position = mul(float4(output.position.xyz, 1.0), g_matProj);

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = texCubemap.Sample(samplerLinearWrap, input.posModel);
    texColor.a = 1.0;
    return texColor;
}
