uint SelectShadowCascadeIndex(float viewDepthPositive)
{
    if (ShadowMapCascadeCount <= 1u)
    {
        return 0u;
    }
    if (viewDepthPositive <= CascadeSplits.x)
    {
        return 0u;
    }
    if (ShadowMapCascadeCount >= 2u && viewDepthPositive <= CascadeSplits.y)
    {
        return 1u;
    }
    if (ShadowMapCascadeCount >= 3u && viewDepthPositive <= CascadeSplits.z)
    {
        return 2u;
    }
    if (ShadowMapCascadeCount >= 4u)
    {
        return 3u;
    }
    return ShadowMapCascadeCount - 1u;
}

float2 ShadowAtlasOriginForCascade(uint cascadeIndex)
{
    return float2(float(cascadeIndex & 1u), float(cascadeIndex / 2u)) * 0.5f;
}

float2 ShadowAtlasUvFromLocal(float2 shadowUvLocal, uint cascadeIndex)
{
    return ShadowAtlasOriginForCascade(cascadeIndex) + shadowUvLocal * 0.5f;
}

float2 ClampShadowAtlasUvToCascadeTile(float2 shadowUvAtlas, uint cascadeIndex)
{
    const float2 atlasOrigin = ShadowAtlasOriginForCascade(cascadeIndex);
    const float2 tileMin = atlasOrigin + InvShadowMapTexelSize;
    const float2 tileMax = atlasOrigin + 0.5f - InvShadowMapTexelSize;
    return clamp(shadowUvAtlas, tileMin, tileMax);
}

float ComputeShadowDepthReference(float LightClipZOverW, float3 WorldNormal, float3 TowardLight)
{
    const float DepthNormalized = saturate(LightClipZOverW);
    const float SlopeTerm = saturate(1.0f - dot(WorldNormal, TowardLight));
    return saturate(DepthNormalized - DepthBiasAndPcfKernel.x - DepthBiasAndPcfKernel.y * SlopeTerm);
}

float SampleShadowPcfClampedToCascade(float2 shadowUvLocal, uint cascadeIndex, float depthReference, int pcfRadius)
{
    float accumulated = 0.0f;
    int sampleCount = 0;

    const float2 atlasCenter = ShadowAtlasUvFromLocal(shadowUvLocal, cascadeIndex);
    for (int offsetV = -pcfRadius; offsetV <= pcfRadius; ++offsetV)
    {
        for (int offsetU = -pcfRadius; offsetU <= pcfRadius; ++offsetU)
        {
            float2 sampleUv = atlasCenter + float2(offsetU, offsetV) * InvShadowMapTexelSize;
            sampleUv = ClampShadowAtlasUvToCascadeTile(sampleUv, cascadeIndex);
            accumulated += ShadowMapDepth.SampleCmpLevelZero(ShadowMapSampler, sampleUv, depthReference);
            sampleCount++;
        }
    }

    return accumulated / max(sampleCount, 1);
}

float ShadowCascadeSplitValue(uint cascadeIndex)
{
    if (cascadeIndex == 0u)
    {
        return CascadeSplits.x;
    }
    if (cascadeIndex == 1u)
    {
        return CascadeSplits.y;
    }
    if (cascadeIndex == 2u)
    {
        return CascadeSplits.z;
    }
    return CascadeSplits.w;
}

float ShadowCascadeTransitionBlend(float viewDepthPositive, uint cascadeIndex)
{
    if (ShadowMapCascadeCount <= 1u || cascadeIndex + 1u >= ShadowMapCascadeCount)
    {
        return 0.0f;
    }

    const float cascadeEnd = ShadowCascadeSplitValue(cascadeIndex);
    const float cascadeStart = cascadeIndex == 0u ? 0.0f : ShadowCascadeSplitValue(cascadeIndex - 1u);
    const float cascadeDepthSpan = max(cascadeEnd - cascadeStart, 1.0f);
    const float transitionDepthSpan = max(cascadeDepthSpan * 0.14f, 1.0f);
    const float transitionStart = cascadeEnd - transitionDepthSpan;
    return saturate((viewDepthPositive - transitionStart) / transitionDepthSpan);
}
