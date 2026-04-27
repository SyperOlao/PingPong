#define MAX_SCENE_LIGHTS 16
#define MAX_SHADOW_CASCADES 4

cbuffer DeferredScreenConstants : register(b0)
{
    matrix InverseProjection;
    matrix InverseView;
    float4 CameraWorldPosition;
    float4 ScreenInverseWidthHeightAndFrameIndex;
};

struct LightGpu
{
    float4 Position;
    float4 Direction;
    float4 Color;
    float4 Parameters;
    float4 Extension;
};

cbuffer LightsConstants : register(b3)
{
    uint TotalLightCount;
    uint3 _pad0;
    LightGpu Lights[MAX_SCENE_LIGHTS];
};

cbuffer ShadowCascadeConstants : register(b4)
{
    matrix LightViewProjection[MAX_SHADOW_CASCADES];
    float4 CascadeSplits;
    uint CascadeCount;
    uint3 _ShadowCascadePadding;
};

cbuffer ShadowSamplingConstants : register(b5)
{
    float4 DepthBiasAndPcfKernel;
    float2 InvShadowMapTexelSize;
    float2 _ShadowSamplingPad0;
    uint ShadowEnabled;
    uint ShadowedDirectionalLightGpuIndex;
    uint ShadowMapCascadeCount;
    uint ShadowCascadeDebugMode;
};

Texture2D AlbedoAmbientTexture : register(t0);
Texture2D NormalReceiveLightingTexture : register(t1);
Texture2D SpecularPowerTexture : register(t2);
Texture2D EmissiveTexture : register(t3);
Texture2D SceneDepthTexture : register(t4);
Texture2D ShadowMapDepth : register(t5);

SamplerState GBufferSampler : register(s0);
SamplerComparisonState ShadowMapSampler : register(s1);

#include "ShadowSamplingCSM.hlsli"

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 Uv : TEXCOORD0;
};

VSOutput VSMain(uint vertexId : SV_VertexID)
{
    VSOutput output;
    const float2 positions[3] =
    {
        float2(-1.0f, -1.0f),
        float2(-1.0f, 3.0f),
        float2(3.0f, -1.0f)
    };
    const float2 uvs[3] =
    {
        float2(0.0f, 1.0f),
        float2(0.0f, -1.0f),
        float2(2.0f, 1.0f)
    };

    output.Position = float4(positions[vertexId], 0.0f, 1.0f);
    output.Uv = uvs[vertexId];
    return output;
}

static const float kDirectionalKind = 0.0f;
static const float kPointKind = 1.0f;
static const float kSpotKind = 2.0f;

float3 DecodeNormal(float4 encodedNormal)
{
    return normalize(encodedNormal.rgb * 2.0f - 1.0f);
}

float3 ReconstructViewPosition(float2 uv, float deviceDepth)
{
    const float4 clipPosition = float4(
        uv.x * 2.0f - 1.0f,
        (1.0f - uv.y) * 2.0f - 1.0f,
        deviceDepth,
        1.0f
    );
    float4 viewPosition = mul(clipPosition, InverseProjection);
    viewPosition.xyz /= max(abs(viewPosition.w), 1.0e-5f);
    return viewPosition.xyz;
}

float3 ReconstructWorldPosition(float3 viewPosition)
{
    const float4 worldPosition = mul(float4(viewPosition, 1.0f), InverseView);
    return worldPosition.xyz / max(abs(worldPosition.w), 1.0e-5f);
}

void EvaluateLight(
    LightGpu light,
    float3 worldPosition,
    float3 worldNormal,
    float3 viewDirection,
    float3 specularColor,
    float specularPower,
    out float3 diffuseContribution,
    out float3 specularContribution
)
{
    diffuseContribution = float3(0.0f, 0.0f, 0.0f);
    specularContribution = float3(0.0f, 0.0f, 0.0f);

    const float3 lightColor = light.Color.rgb;
    float3 lightVector = float3(0.0f, 0.0f, 0.0f);
    float attenuation = 1.0f;

    const float kind = light.Parameters.x;

    if (abs(kind - kDirectionalKind) < 0.01f)
    {
        lightVector = normalize(light.Direction.xyz);
    }
    else if (abs(kind - kPointKind) < 0.01f)
    {
        const float3 toLight = light.Position.xyz - worldPosition;
        const float distanceToLight = length(toLight);
        lightVector = toLight / max(distanceToLight, 0.0001f);
        const float range = light.Parameters.y;
        attenuation = saturate(1.0f - distanceToLight / max(range, 0.0001f));
        attenuation *= attenuation;
    }
    else if (abs(kind - kSpotKind) < 0.01f)
    {
        const float3 toLight = light.Position.xyz - worldPosition;
        const float distanceToLight = length(toLight);
        lightVector = toLight / max(distanceToLight, 0.0001f);
        const float range = light.Parameters.y;
        attenuation = saturate(1.0f - distanceToLight / max(range, 0.0001f));
        attenuation *= attenuation;

        const float3 axis = normalize(light.Direction.xyz);
        const float3 fromLightToPoint = normalize(worldPosition - light.Position.xyz);
        const float cosAngle = dot(axis, fromLightToPoint);
        const float cosInner = light.Parameters.z;
        const float cosOuter = light.Parameters.w;
        const float spotFactor = saturate((cosAngle - cosOuter) / max(cosInner - cosOuter, 0.0001f));
        attenuation *= spotFactor;
    }

    const float normalDotLight = saturate(dot(worldNormal, lightVector));
    diffuseContribution = lightColor * normalDotLight * attenuation;

    if (normalDotLight > 0.0f)
    {
        const float3 reflection = reflect(-lightVector, worldNormal);
        const float specularTerm = pow(saturate(dot(reflection, viewDirection)), max(specularPower, 1.0f));
        specularContribution = specularColor * lightColor * specularTerm * attenuation;
    }
}

float DirectionalShadowAttenuation(float3 worldPosition, float3 worldNormal, float viewDepthPositive)
{
    if (ShadowEnabled == 0u || ShadowMapCascadeCount == 0u)
    {
        return 1.0f;
    }
    if (ShadowedDirectionalLightGpuIndex >= TotalLightCount)
    {
        return 1.0f;
    }

    const float3 towardLight = normalize(Lights[ShadowedDirectionalLightGpuIndex].Direction.xyz);
    const float3 biasedWorldPosition = worldPosition + worldNormal * DepthBiasAndPcfKernel.z;
    const uint cascadeIndexPrimary = SelectShadowCascadeIndex(viewDepthPositive);

    const float4 lightClipPrimary = mul(float4(biasedWorldPosition, 1.0f), LightViewProjection[cascadeIndexPrimary]);
    const float inverseWPrimary = 1.0f / max(abs(lightClipPrimary.w), 1.0e-5f);
    const float3 ndcPrimary = lightClipPrimary.xyz * inverseWPrimary;
    const float2 shadowUvLocalPrimary = float2(ndcPrimary.x * 0.5f + 0.5f, -ndcPrimary.y * 0.5f + 0.5f);

    if (ndcPrimary.z < 0.0f || ndcPrimary.z > 1.0f
        || shadowUvLocalPrimary.x < 0.0f || shadowUvLocalPrimary.x > 1.0f
        || shadowUvLocalPrimary.y < 0.0f || shadowUvLocalPrimary.y > 1.0f)
    {
        return 1.0f;
    }

    const int pcfRadius = clamp((int)DepthBiasAndPcfKernel.w, 1, 4);
    const float depthReferencePrimary = ComputeShadowDepthReference(saturate(ndcPrimary.z), worldNormal, towardLight);
    const float shadowPrimary = SampleShadowPcfClampedToCascade(
        shadowUvLocalPrimary,
        cascadeIndexPrimary,
        depthReferencePrimary,
        pcfRadius
    );

    const float cascadeBlendFactor = ShadowCascadeTransitionBlend(viewDepthPositive, cascadeIndexPrimary);
    if (cascadeBlendFactor <= 0.0f || cascadeIndexPrimary + 1u >= ShadowMapCascadeCount)
    {
        return shadowPrimary;
    }

    const uint cascadeIndexSecondary = cascadeIndexPrimary + 1u;
    const float4 lightClipSecondary = mul(float4(biasedWorldPosition, 1.0f), LightViewProjection[cascadeIndexSecondary]);
    const float inverseWSecondary = 1.0f / max(abs(lightClipSecondary.w), 1.0e-5f);
    const float3 ndcSecondary = lightClipSecondary.xyz * inverseWSecondary;
    const float2 shadowUvLocalSecondary = float2(ndcSecondary.x * 0.5f + 0.5f, -ndcSecondary.y * 0.5f + 0.5f);

    if (ndcSecondary.z < 0.0f || ndcSecondary.z > 1.0f
        || shadowUvLocalSecondary.x < 0.0f || shadowUvLocalSecondary.x > 1.0f
        || shadowUvLocalSecondary.y < 0.0f || shadowUvLocalSecondary.y > 1.0f)
    {
        return shadowPrimary;
    }

    const float depthReferenceSecondary = ComputeShadowDepthReference(saturate(ndcSecondary.z), worldNormal, towardLight);
    const float shadowSecondary = SampleShadowPcfClampedToCascade(
        shadowUvLocalSecondary,
        cascadeIndexSecondary,
        depthReferenceSecondary,
        pcfRadius
    );

    return lerp(shadowPrimary, shadowSecondary, cascadeBlendFactor);
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    const float sceneDepth = SceneDepthTexture.Sample(GBufferSampler, input.Uv).r;
    if (sceneDepth >= 0.999999f)
    {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    const float4 albedoAmbient = AlbedoAmbientTexture.Sample(GBufferSampler, input.Uv);
    const float4 normalReceive = NormalReceiveLightingTexture.Sample(GBufferSampler, input.Uv);
    const float4 specularPowerPacked = SpecularPowerTexture.Sample(GBufferSampler, input.Uv);
    const float3 emissive = EmissiveTexture.Sample(GBufferSampler, input.Uv).rgb;

    const float3 albedo = albedoAmbient.rgb;
    const float ambientFactor = albedoAmbient.a;
    const float3 worldNormal = DecodeNormal(normalReceive);
    const float receiveLighting = normalReceive.a;
    const float3 specularColor = specularPowerPacked.rgb;
    const float specularPower = max(specularPowerPacked.a * 256.0f, 1.0f);

    const float3 viewPosition = ReconstructViewPosition(input.Uv, sceneDepth);
    const float3 worldPosition = ReconstructWorldPosition(viewPosition);
    const float3 viewDirection = normalize(CameraWorldPosition.xyz - worldPosition);
    const float viewDepthPositive = max(-viewPosition.z, 0.0f);

    if (receiveLighting < 0.5f)
    {
        return float4(saturate(albedo + emissive), 1.0f);
    }

    const float directionalShadow = DirectionalShadowAttenuation(worldPosition, worldNormal, viewDepthPositive);

    if (ShadowCascadeDebugMode != 0u && ShadowEnabled != 0u && ShadowMapCascadeCount > 0u)
    {
        const uint cascadeDebugIndex = SelectShadowCascadeIndex(viewDepthPositive);
        const float3 cascadeColors[4] =
        {
            float3(0.85f, 0.25f, 0.2f),
            float3(0.25f, 0.75f, 0.3f),
            float3(0.25f, 0.35f, 0.9f),
            float3(0.9f, 0.85f, 0.2f)
        };
        return float4(saturate(albedo * cascadeColors[cascadeDebugIndex] * directionalShadow + emissive), 1.0f);
    }

    float3 diffuseAccumulation = float3(0.0f, 0.0f, 0.0f);
    float3 specularAccumulation = float3(0.0f, 0.0f, 0.0f);

    const uint lightCount = min(TotalLightCount, MAX_SCENE_LIGHTS);
    for (uint lightIndex = 0u; lightIndex < lightCount; ++lightIndex)
    {
        float3 diffuseContribution = float3(0.0f, 0.0f, 0.0f);
        float3 specularContribution = float3(0.0f, 0.0f, 0.0f);
        EvaluateLight(
            Lights[lightIndex],
            worldPosition,
            worldNormal,
            viewDirection,
            specularColor,
            specularPower,
            diffuseContribution,
            specularContribution
        );

        float shadowFactor = 1.0f;
        if (ShadowEnabled != 0u
            && lightIndex == ShadowedDirectionalLightGpuIndex
            && abs(Lights[lightIndex].Parameters.x - kDirectionalKind) < 0.01f)
        {
            shadowFactor = directionalShadow;
        }

        diffuseAccumulation += diffuseContribution * shadowFactor;
        specularAccumulation += specularContribution * shadowFactor;
    }

    const float shadowVisibilityForDirectLighting =
        ShadowEnabled != 0u ? lerp(0.38f, 1.0f, directionalShadow) : 1.0f;
    const float shadowVisibilityForAmbient =
        ShadowEnabled != 0u ? lerp(0.72f, 1.0f, directionalShadow) : 1.0f;
    const float3 ambientTerm = albedo * ambientFactor * shadowVisibilityForAmbient;
    const float3 litColor =
        ambientTerm
        + emissive
        + diffuseAccumulation * albedo * shadowVisibilityForDirectLighting
        + specularAccumulation * shadowVisibilityForDirectLighting;
    return float4(litColor, 1.0f);
}
