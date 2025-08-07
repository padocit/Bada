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
    
    matrix matViewProj = mul(g_matView, g_matProj);
    matrix matWorldViewProj = mul(g_matWorld, matViewProj);

    output.posModel = input.pos.xyz;
    output.position = mul(input.pos, matWorldViewProj);

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = texCubemap.Sample(samplerLinearWrap, input.posModel);
    texColor.a = 1.0;
    return texColor;
}
