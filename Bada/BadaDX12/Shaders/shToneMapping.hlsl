Texture2D<float4> HDRTexture : register(t0);
SamplerState LinearSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

// Ǯ��ũ�� �ﰢ�� ����
PSInput VSMain(uint vertexId : SV_VertexID)
{
    PSInput result;
    
    // 3�� �������� Ǯ��ũ�� �ﰢ�� ���� (ClockWise)
    float2 texcoord = float2(vertexId & 2, (vertexId << 1) & 2);
    result.position = float4(texcoord * 2.0f - 1.0f, 0.0f, 1.0f);
    result.texcoord = float2(texcoord.x, 1.0f - texcoord.y); // Y ��ǥ ������
    
    return result;
}

// ������ ����� (Reinhard)
float4 PSMain(PSInput input) : SV_Target
{
    float3 hdrColor = HDRTexture.Sample(LinearSampler, input.texcoord).rgb;
    
    float3 invGamma = float3(1, 1, 1) / 2.2;
    hdrColor = pow(hdrColor, invGamma);
  
    return float4(hdrColor, 1.0f);
}