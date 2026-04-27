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

cbuffer ParticleUpdateConstants : register(b0)
{
    matrix ViewProjection;
    matrix InverseProjection;
    matrix InverseView;
    float4 EmitterCenterAndDeltaTime;
    float4 EmitterExtentsAndLifetime;
    float4 VelocityMinAndStartSize;
    float4 VelocityMaxAndCollisionDamping;
    float4 GravityAndCollisionBias;
    float4 ParticleColor;
    float4 CameraWorldPosition;
    float4 DepthTextureSizeAndCollisionEnabled;
    uint MaxParticles;
    uint SpawnStartIndex;
    uint SpawnCount;
    uint FrameIndex;
};

Texture2D<float> SceneDepthTexture : register(t0);
SamplerState DepthSampler : register(s0);

RWStructuredBuffer<ParticleGpu> Particles : register(u0);
RWStructuredBuffer<ParticleSortKey> SortKeys : register(u1);

uint Hash(uint value)
{
    value ^= value >> 16u;
    value *= 0x7feb352du;
    value ^= value >> 15u;
    value *= 0x846ca68bu;
    value ^= value >> 16u;
    return value;
}

float Random01(uint seed)
{
    return (float)(Hash(seed) & 0x00ffffffu) / 16777215.0f;
}

float3 Random3(uint seed)
{
    return float3(
        Random01(seed),
        Random01(seed + 1299721u),
        Random01(seed + 4256249u)
    );
}

bool IsIndexInSpawnWindow(uint particleIndex)
{
    if (SpawnCount == 0u)
    {
        return false;
    }

    if (SpawnCount >= MaxParticles)
    {
        return true;
    }

    const uint wrappedOffset = (particleIndex + MaxParticles - SpawnStartIndex) & (MaxParticles - 1u);
    return wrappedOffset < SpawnCount;
}

float3 ReconstructWorldPosition(float2 uv, float deviceDepth)
{
    const float4 clipPosition = float4(
        uv.x * 2.0f - 1.0f,
        (1.0f - uv.y) * 2.0f - 1.0f,
        deviceDepth,
        1.0f
    );
    float4 viewPosition = mul(clipPosition, InverseProjection);
    viewPosition.xyz /= max(abs(viewPosition.w), 1.0e-5f);

    float4 worldPosition = mul(float4(viewPosition.xyz, 1.0f), InverseView);
    worldPosition.xyz /= max(abs(worldPosition.w), 1.0e-5f);
    return worldPosition.xyz;
}

float3 EstimateDepthNormal(float2 uv, float centerDepth)
{
    const float2 texel = DepthTextureSizeAndCollisionEnabled.xy;
    const float2 uvRight = saturate(uv + float2(texel.x, 0.0f));
    const float2 uvUp = saturate(uv + float2(0.0f, -texel.y));
    const float rightDepth = SceneDepthTexture.SampleLevel(DepthSampler, uvRight, 0.0f);
    const float upDepth = SceneDepthTexture.SampleLevel(DepthSampler, uvUp, 0.0f);

    const float3 centerWorld = ReconstructWorldPosition(uv, centerDepth);
    const float3 rightWorld = ReconstructWorldPosition(uvRight, rightDepth);
    const float3 upWorld = ReconstructWorldPosition(uvUp, upDepth);

    float3 normal = normalize(cross(upWorld - centerWorld, rightWorld - centerWorld));
    const float3 toCamera = normalize(CameraWorldPosition.xyz - centerWorld);
    if (dot(normal, toCamera) < 0.0f)
    {
        normal = -normal;
    }
    return normal;
}

void ResolveDepthCollision(inout float3 position, inout float3 velocity, float particleRadius)
{
    if (DepthTextureSizeAndCollisionEnabled.z < 0.5f)
    {
        return;
    }

    const float4 clipPosition = mul(float4(position, 1.0f), ViewProjection);
    if (clipPosition.w <= 1.0e-5f)
    {
        return;
    }

    const float3 ndc = clipPosition.xyz / clipPosition.w;
    if (ndc.z < 0.0f || ndc.z > 1.0f)
    {
        return;
    }

    const float2 uv = float2(ndc.x * 0.5f + 0.5f, -ndc.y * 0.5f + 0.5f);
    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f)
    {
        return;
    }

    const float particleDepth = saturate(ndc.z);
    const float sceneDepth = SceneDepthTexture.SampleLevel(DepthSampler, uv, 0.0f);
    if (sceneDepth >= 0.999999f)
    {
        return;
    }

    const float depthBias = GravityAndCollisionBias.w;
    if (particleDepth <= sceneDepth + depthBias)
    {
        return;
    }

    const float3 surfaceWorld = ReconstructWorldPosition(uv, sceneDepth);
    const float3 normal = EstimateDepthNormal(uv, sceneDepth);
    position = surfaceWorld + normal * max(particleRadius, 0.01f);

    if (dot(velocity, normal) < 0.0f)
    {
        velocity = reflect(velocity, normal) * VelocityMaxAndCollisionDamping.w;
    }
}

ParticleGpu SpawnParticle(uint particleIndex)
{
    const uint seed = particleIndex * 747796405u + FrameIndex * 2891336453u;
    const float3 randomPosition = Random3(seed) * 2.0f - 1.0f;
    const float3 randomVelocity = Random3(seed + 31337u);
    const float lifetimeJitter = lerp(0.82f, 1.18f, Random01(seed + 811u));

    ParticleGpu particle;
    particle.PositionLife = float4(
        EmitterCenterAndDeltaTime.xyz + randomPosition * EmitterExtentsAndLifetime.xyz,
        EmitterExtentsAndLifetime.w * lifetimeJitter
    );
    particle.VelocitySize = float4(
        lerp(VelocityMinAndStartSize.xyz, VelocityMaxAndCollisionDamping.xyz, randomVelocity),
        VelocityMinAndStartSize.w
    );
    particle.Color = ParticleColor;
    return particle;
}

[numthreads(256, 1, 1)]
void CSMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint particleIndex = dispatchThreadId.x;
    if (particleIndex >= MaxParticles)
    {
        return;
    }

    ParticleGpu particle = Particles[particleIndex];

    if (IsIndexInSpawnWindow(particleIndex))
    {
        particle = SpawnParticle(particleIndex);
    }
    else if (particle.PositionLife.w > 0.0f)
    {
        const float deltaTime = EmitterCenterAndDeltaTime.w;
        float3 position = particle.PositionLife.xyz;
        float3 velocity = particle.VelocitySize.xyz;
        const float size = max(particle.VelocitySize.w, 0.001f);

        velocity += GravityAndCollisionBias.xyz * deltaTime;
        position += velocity * deltaTime;
        ResolveDepthCollision(position, velocity, size * 0.5f);

        particle.PositionLife.xyz = position;
        particle.PositionLife.w -= deltaTime;
        particle.VelocitySize.xyz = velocity;
    }

    const bool alive = particle.PositionLife.w > 0.0f;
    Particles[particleIndex] = particle;

    ParticleSortKey sortKey;
    const float3 toCamera = particle.PositionLife.xyz - CameraWorldPosition.xyz;
    sortKey.Key = alive ? -dot(toCamera, toCamera) : 3.402823466e+38f;
    sortKey.Index = particleIndex;
    sortKey.Alive = alive ? 1u : 0u;
    sortKey.Padding = 0u;
    SortKeys[particleIndex] = sortKey;
}
