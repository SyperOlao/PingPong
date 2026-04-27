struct ParticleGpu
{
    float4 PositionLife;
    float4 VelocitySize;
    float4 Color;
};

struct ParticleSortKey
{
    float Key;
    uint Index;
    uint Alive;
    uint Padding;
};

cbuffer ParticleDrawConstants : register(b0)
{
    matrix ViewProjection;
    float4 CameraRightAndSizeScale;
    float4 CameraUpAndAlphaScale;
    float4 ScreenInverseSizeAndDepthBias;
};

StructuredBuffer<ParticleGpu> Particles : register(t0);
StructuredBuffer<ParticleSortKey> SortKeys : register(t1);
Texture2D<float> SceneDepthTexture : register(t2);
SamplerState DepthSampler : register(s0);

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 Uv : TEXCOORD0;
    float4 Color : COLOR;
    float DeviceDepth : TEXCOORD1;
};

VSOutput VSMain(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID)
{
    const float2 corners[6] =
    {
        float2(-1.0f, -1.0f),
        float2(-1.0f, 1.0f),
        float2(1.0f, 1.0f),
        float2(-1.0f, -1.0f),
        float2(1.0f, 1.0f),
        float2(1.0f, -1.0f)
    };

    const ParticleSortKey sortKey = SortKeys[instanceId];
    const ParticleGpu particle = Particles[sortKey.Index];
    const bool alive = sortKey.Alive != 0u && particle.PositionLife.w > 0.0f;
    const float2 corner = corners[vertexId];
    const float size = alive ? particle.VelocitySize.w * CameraRightAndSizeScale.w : 0.0f;

    const float3 worldPosition =
        particle.PositionLife.xyz
        + CameraRightAndSizeScale.xyz * corner.x * size
        + CameraUpAndAlphaScale.xyz * corner.y * size;

    VSOutput output;
    output.Position = mul(float4(worldPosition, 1.0f), ViewProjection);
    output.Uv = corner * 0.5f + 0.5f;
    output.Color = particle.Color;
    output.Color.a *= alive ? CameraUpAndAlphaScale.w : 0.0f;
    output.DeviceDepth = output.Position.z / max(output.Position.w, 1.0e-5f);
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    const float2 centerVector = input.Uv * 2.0f - 1.0f;
    const float radiusSquared = dot(centerVector, centerVector);
    if (radiusSquared > 1.0f || input.Color.a <= 0.0f)
    {
        discard;
    }

    if (ScreenInverseSizeAndDepthBias.w > 0.5f)
    {
        const float2 screenUv = input.Position.xy * ScreenInverseSizeAndDepthBias.xy;
        const float sceneDepth = SceneDepthTexture.SampleLevel(DepthSampler, screenUv, 0.0f);
        if (sceneDepth < 0.999999f && input.DeviceDepth > sceneDepth + ScreenInverseSizeAndDepthBias.z)
        {
            discard;
        }
    }

    const float softAlpha = saturate(1.0f - radiusSquared);
    float4 color = input.Color;
    color.a *= softAlpha * softAlpha;
    return color;
}
