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
    float4 MaterialParameters;
};

Texture2D DiffuseTexture : register(t0);
SamplerState DiffuseSampler : register(s0);

struct VSInputVertexColor
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
};

struct VSInputTextured
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 WorldNormal : NORMAL;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
};

VSOutput VSMainVertexColor(VSInputVertexColor input)
{
    VSOutput output;

    const float4 worldPosition = mul(float4(input.Position, 1.0f), World);
    output.Position = mul(worldPosition, ViewProjection);

    const float3x3 worldInverseTranspose = (float3x3)WorldInverseTranspose;
    output.WorldNormal = normalize(mul(input.Normal, worldInverseTranspose));
    output.Color = input.Color;
    output.TexCoord = float2(0.0f, 0.0f);

    return output;
}

VSOutput VSMainTextured(VSInputTextured input)
{
    VSOutput output;

    const float4 worldPosition = mul(float4(input.Position, 1.0f), World);
    output.Position = mul(worldPosition, ViewProjection);

    const float3x3 worldInverseTranspose = (float3x3)WorldInverseTranspose;
    output.WorldNormal = normalize(mul(input.Normal, worldInverseTranspose));
    output.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    output.TexCoord = input.TexCoord;

    return output;
}

struct GBufferOutput
{
    float4 AlbedoAmbient : SV_Target0;
    float4 NormalReceiveLighting : SV_Target1;
    float4 SpecularPower : SV_Target2;
};

float3 EncodeNormal(float3 worldNormal)
{
    return normalize(worldNormal) * 0.5f + 0.5f;
}

GBufferOutput PackGBuffer(VSOutput input, float3 textureColor)
{
    GBufferOutput output;

    const float3 albedo = saturate(textureColor * input.Color.rgb * BaseColor.rgb);
    const float ambientFactor = saturate(EmissiveAndAmbient.w);
    const float receiveLighting = MaterialParameters.x >= 0.5f ? 1.0f : 0.0f;
    const float encodedSpecularPower = saturate(max(SpecularColorAndPower.w, 1.0f) / 256.0f);

    output.AlbedoAmbient = float4(albedo, ambientFactor);
    output.NormalReceiveLighting = float4(EncodeNormal(input.WorldNormal), receiveLighting);
    output.SpecularPower = float4(saturate(SpecularColorAndPower.rgb), encodedSpecularPower);

    return output;
}

GBufferOutput PSMainVertexColor(VSOutput input)
{
    return PackGBuffer(input, float3(1.0f, 1.0f, 1.0f));
}

GBufferOutput PSMainTextured(VSOutput input)
{
    const float3 textureColor = DiffuseTexture.Sample(DiffuseSampler, input.TexCoord).rgb;
    return PackGBuffer(input, textureColor);
}
