#define MAX_SCENE_LIGHTS 8

cbuffer CameraConstants : register(b0)
{
    matrix View;
    matrix Projection;
    matrix ViewProjection;
    float4 CameraWorldPosition;
};

cbuffer ObjectConstants : register(b1)
{
    matrix World;
    matrix WorldInverseTranspose;
};

cbuffer MaterialConstants : register(b2)
{
    float4 BaseColor;
    float4 SpecularColorAndPower;
    float4 EmissiveAndAmbient;
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

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float3 WorldNormal : NORMAL;
    float3 WorldPosition : TEXCOORD0;
    float4 Color : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput output;

    float4 worldPosition = mul(float4(input.Position, 1.0f), World);
    output.WorldPosition = worldPosition.xyz;

    float3x3 worldInverseTranspose = (float3x3)WorldInverseTranspose;
    output.WorldNormal = normalize(mul(input.Normal, worldInverseTranspose));

    output.Position = mul(worldPosition, ViewProjection);
    output.Color = input.Color;

    return output;
}

static const float kDirectionalKind = 0.0f;
static const float kPointKind = 1.0f;
static const float kSpotKind = 2.0f;

void EvaluateLight(
    LightGpu light,
    float3 worldPosition,
    float3 worldNormal,
    float3 viewDirection,
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
        attenuation = attenuation * attenuation;
    }
    else if (abs(kind - kSpotKind) < 0.01f)
    {
        const float3 toLight = light.Position.xyz - worldPosition;
        const float distanceToLight = length(toLight);
        lightVector = toLight / max(distanceToLight, 0.0001f);
        const float range = light.Parameters.y;
        attenuation = saturate(1.0f - distanceToLight / max(range, 0.0001f));
        attenuation = attenuation * attenuation;

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

    const float3 reflection = reflect(-lightVector, worldNormal);
    const float specularPower = max(SpecularColorAndPower.w, 1.0f);
    const float specularTerm = pow(saturate(dot(reflection, viewDirection)), specularPower);
    specularContribution = SpecularColorAndPower.rgb * lightColor * specularTerm * attenuation;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    const float3 worldNormal = normalize(input.WorldNormal);
    const float3 viewDirection = normalize(CameraWorldPosition.xyz - input.WorldPosition);

    float3 diffuseAccumulation = float3(0.0f, 0.0f, 0.0f);
    float3 specularAccumulation = float3(0.0f, 0.0f, 0.0f);

    const uint lightCount = min(TotalLightCount, MAX_SCENE_LIGHTS);

    for (uint lightIndex = 0u; lightIndex < lightCount; ++lightIndex)
    {
        float3 diffuseContribution = float3(0.0f, 0.0f, 0.0f);
        float3 specularContribution = float3(0.0f, 0.0f, 0.0f);
        EvaluateLight(
            Lights[lightIndex],
            input.WorldPosition,
            worldNormal,
            viewDirection,
            diffuseContribution,
            specularContribution
        );
        diffuseAccumulation += diffuseContribution;
        specularAccumulation += specularContribution;
    }

    const float3 baseSample = input.Color.rgb * BaseColor.rgb;
    const float ambientScale = EmissiveAndAmbient.w;
    const float3 ambientTerm = baseSample * ambientScale;
    const float3 emissiveTerm = EmissiveAndAmbient.xyz;

    const float3 litColor = ambientTerm + emissiveTerm + diffuseAccumulation * baseSample + specularAccumulation;

    return float4(litColor, BaseColor.a * input.Color.a);
}
