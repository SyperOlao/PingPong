#include "Core/Graphics/Particles/GpuParticleSystem.h"

#include "Core/Graphics/Camera.h"
#include "Core/Graphics/D3d11Helpers.h"
#include "Core/Graphics/GraphicsDevice.h"
#include "Core/Graphics/Rendering/Lighting/SceneLighting3D.h"
#include "Core/Graphics/ShaderCompiler.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace
{
    constexpr UINT kParticleThreadGroupSize = 256u;
    constexpr std::uint32_t kMinimumParticleCapacity = 256u;

    void CreateStructuredBuffer(
        ID3D11Device *const device,
        const UINT elementByteWidth,
        const UINT elementCount,
        const void *const initialData,
        Microsoft::WRL::ComPtr<ID3D11Buffer> &buffer
    )
    {
        D3D11_BUFFER_DESC description{};
        description.Usage = D3D11_USAGE_DEFAULT;
        description.ByteWidth = elementByteWidth * elementCount;
        description.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        description.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        description.StructureByteStride = elementByteWidth;

        D3D11_SUBRESOURCE_DATA subresource{};
        subresource.pSysMem = initialData;

        D3d11Helpers::ThrowIfFailed(
            device->CreateBuffer(&description, initialData != nullptr ? &subresource : nullptr, buffer.GetAddressOf()),
            "GpuParticleSystem failed to create structured buffer."
        );
    }

    void CreateDynamicConstantBuffer(
        ID3D11Device *const device,
        const UINT byteWidth,
        Microsoft::WRL::ComPtr<ID3D11Buffer> &buffer,
        const char *const errorMessage
    )
    {
        D3D11_BUFFER_DESC description{};
        description.Usage = D3D11_USAGE_DYNAMIC;
        description.ByteWidth = byteWidth;
        description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3d11Helpers::ThrowIfFailed(
            device->CreateBuffer(&description, nullptr, buffer.GetAddressOf()),
            errorMessage
        );
    }

    void UploadConstantBuffer(
        ID3D11DeviceContext *const context,
        ID3D11Buffer *const buffer,
        const void *const data,
        const std::size_t byteCount,
        const char *const errorMessage
    )
    {
        D3D11_MAPPED_SUBRESOURCE mapped{};
        D3d11Helpers::ThrowIfFailed(
            context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
            errorMessage
        );
        std::memcpy(mapped.pData, data, byteCount);
        context->Unmap(buffer, 0);
    }
}

GpuParticleSystem::~GpuParticleSystem()
{
    Shutdown();
}

void GpuParticleSystem::Initialize(GraphicsDevice &graphics, const std::uint32_t maxParticles)
{
    ID3D11Device *const device = graphics.GetDevice();
    if (device == nullptr)
    {
        throw std::logic_error("GpuParticleSystem::Initialize requires a valid D3D11 device.");
    }

    Shutdown();

    m_maxParticles = (std::max)(RoundDownToPowerOfTwo(maxParticles), kMinimumParticleCapacity);
    m_commonStates = std::make_unique<DirectX::CommonStates>(device);

    CreateShaders(device);
    CreateBuffers(device);
    CreateViews(device);
    CreateConstantBuffers(device);
    CreateSampler(device);
}

void GpuParticleSystem::Shutdown() noexcept
{
    m_commonStates.reset();
    m_depthSampler.Reset();
    m_drawPixelShader.Reset();
    m_drawVertexShader.Reset();
    m_sortComputeShader.Reset();
    m_updateComputeShader.Reset();
    m_drawConstantBuffer.Reset();
    m_sortConstantBuffer.Reset();
    m_updateConstantBuffer.Reset();
    m_sortKeyUnorderedAccessView.Reset();
    m_particleUnorderedAccessView.Reset();
    m_sortKeyShaderResourceView.Reset();
    m_particleShaderResourceView.Reset();
    m_sortKeyBuffer.Reset();
    m_particleBuffer.Reset();
    m_maxParticles = 0u;
    m_spawnCursor = 0u;
    m_frameIndex = 0u;
    m_spawnAccumulator = 0.0f;
}

void GpuParticleSystem::SetEmitterDesc(const GpuParticleEmitterDesc &desc) noexcept
{
    m_emitter = desc;
}

const GpuParticleEmitterDesc &GpuParticleSystem::GetEmitterDesc() const noexcept
{
    return m_emitter;
}

GpuParticleEmitterDesc &GpuParticleSystem::GetMutableEmitterDesc() noexcept
{
    return m_emitter;
}

std::uint32_t GpuParticleSystem::GetMaxParticles() const noexcept
{
    return m_maxParticles;
}

bool GpuParticleSystem::IsInitialized() const noexcept
{
    return m_particleBuffer != nullptr
        && m_sortKeyBuffer != nullptr
        && m_updateComputeShader != nullptr
        && m_sortComputeShader != nullptr
        && m_drawVertexShader != nullptr
        && m_drawPixelShader != nullptr;
}

void GpuParticleSystem::SimulateAndRender(
    GraphicsDevice &graphics,
    const Camera &camera,
    const float aspectRatio,
    const float deltaTimeSeconds,
    ID3D11ShaderResourceView *const sceneDepthShaderResourceView
)
{
    if (!IsInitialized() || !m_emitter.Enabled)
    {
        return;
    }

    Simulate(graphics, camera, aspectRatio, deltaTimeSeconds, sceneDepthShaderResourceView);
    Render(graphics, camera, aspectRatio, sceneDepthShaderResourceView);
}

void GpuParticleSystem::CreateShaders(ID3D11Device *const device)
{
    const auto updateBlob = ShaderCompiler::CompileFromFile(
        "Core/Shaders/GpuParticlesUpdateCS.hlsl",
        "CSMain",
        "cs_5_0"
    );
    const auto sortBlob = ShaderCompiler::CompileFromFile(
        "Core/Shaders/GpuParticlesBitonicSortCS.hlsl",
        "CSMain",
        "cs_5_0"
    );
    const auto drawVertexBlob = ShaderCompiler::CompileFromFile(
        "Core/Shaders/GpuParticlesDraw3D.hlsl",
        "VSMain",
        "vs_5_0"
    );
    const auto drawPixelBlob = ShaderCompiler::CompileFromFile(
        "Core/Shaders/GpuParticlesDraw3D.hlsl",
        "PSMain",
        "ps_5_0"
    );

    D3d11Helpers::ThrowIfFailed(
        device->CreateComputeShader(
            updateBlob->GetBufferPointer(),
            updateBlob->GetBufferSize(),
            nullptr,
            m_updateComputeShader.GetAddressOf()
        ),
        "GpuParticleSystem failed to create update compute shader."
    );
    D3d11Helpers::ThrowIfFailed(
        device->CreateComputeShader(
            sortBlob->GetBufferPointer(),
            sortBlob->GetBufferSize(),
            nullptr,
            m_sortComputeShader.GetAddressOf()
        ),
        "GpuParticleSystem failed to create bitonic sort compute shader."
    );
    D3d11Helpers::ThrowIfFailed(
        device->CreateVertexShader(
            drawVertexBlob->GetBufferPointer(),
            drawVertexBlob->GetBufferSize(),
            nullptr,
            m_drawVertexShader.GetAddressOf()
        ),
        "GpuParticleSystem failed to create draw vertex shader."
    );
    D3d11Helpers::ThrowIfFailed(
        device->CreatePixelShader(
            drawPixelBlob->GetBufferPointer(),
            drawPixelBlob->GetBufferSize(),
            nullptr,
            m_drawPixelShader.GetAddressOf()
        ),
        "GpuParticleSystem failed to create draw pixel shader."
    );
}

void GpuParticleSystem::CreateBuffers(ID3D11Device *const device)
{
    std::vector<GpuParticleData> particles(m_maxParticles);
    std::vector<GpuParticleSortKey> sortKeys(m_maxParticles);

    for (std::uint32_t index = 0u; index < m_maxParticles; ++index)
    {
        particles[index].PositionLife = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, -1.0f);
        particles[index].VelocitySize = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
        particles[index].Color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f);
        sortKeys[index].Key = std::numeric_limits<float>::max();
        sortKeys[index].Index = index;
        sortKeys[index].Alive = 0u;
    }

    CreateStructuredBuffer(
        device,
        static_cast<UINT>(sizeof(GpuParticleData)),
        m_maxParticles,
        particles.data(),
        m_particleBuffer
    );
    CreateStructuredBuffer(
        device,
        static_cast<UINT>(sizeof(GpuParticleSortKey)),
        m_maxParticles,
        sortKeys.data(),
        m_sortKeyBuffer
    );
}

void GpuParticleSystem::CreateViews(ID3D11Device *const device)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription{};
    shaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    shaderResourceViewDescription.Buffer.FirstElement = 0u;
    shaderResourceViewDescription.Buffer.NumElements = m_maxParticles;
    shaderResourceViewDescription.Format = DXGI_FORMAT_UNKNOWN;

    D3d11Helpers::ThrowIfFailed(
        device->CreateShaderResourceView(
            m_particleBuffer.Get(),
            &shaderResourceViewDescription,
            m_particleShaderResourceView.GetAddressOf()
        ),
        "GpuParticleSystem failed to create particle SRV."
    );
    D3d11Helpers::ThrowIfFailed(
        device->CreateShaderResourceView(
            m_sortKeyBuffer.Get(),
            &shaderResourceViewDescription,
            m_sortKeyShaderResourceView.GetAddressOf()
        ),
        "GpuParticleSystem failed to create sort-key SRV."
    );

    D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDescription{};
    unorderedAccessViewDescription.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    unorderedAccessViewDescription.Buffer.FirstElement = 0u;
    unorderedAccessViewDescription.Buffer.NumElements = m_maxParticles;
    unorderedAccessViewDescription.Format = DXGI_FORMAT_UNKNOWN;

    D3d11Helpers::ThrowIfFailed(
        device->CreateUnorderedAccessView(
            m_particleBuffer.Get(),
            &unorderedAccessViewDescription,
            m_particleUnorderedAccessView.GetAddressOf()
        ),
        "GpuParticleSystem failed to create particle UAV."
    );
    D3d11Helpers::ThrowIfFailed(
        device->CreateUnorderedAccessView(
            m_sortKeyBuffer.Get(),
            &unorderedAccessViewDescription,
            m_sortKeyUnorderedAccessView.GetAddressOf()
        ),
        "GpuParticleSystem failed to create sort-key UAV."
    );
}

void GpuParticleSystem::CreateConstantBuffers(ID3D11Device *const device)
{
    CreateDynamicConstantBuffer(
        device,
        static_cast<UINT>(sizeof(GpuParticleUpdateConstants)),
        m_updateConstantBuffer,
        "GpuParticleSystem failed to create update constant buffer."
    );
    CreateDynamicConstantBuffer(
        device,
        static_cast<UINT>(sizeof(GpuParticleSortConstants)),
        m_sortConstantBuffer,
        "GpuParticleSystem failed to create sort constant buffer."
    );
    CreateDynamicConstantBuffer(
        device,
        static_cast<UINT>(sizeof(GpuParticleDrawConstants)),
        m_drawConstantBuffer,
        "GpuParticleSystem failed to create draw constant buffer."
    );
}

void GpuParticleSystem::CreateSampler(ID3D11Device *const device)
{
    D3D11_SAMPLER_DESC description{};
    description.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    description.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    description.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    description.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    description.MinLOD = 0.0f;
    description.MaxLOD = D3D11_FLOAT32_MAX;

    D3d11Helpers::ThrowIfFailed(
        device->CreateSamplerState(&description, m_depthSampler.GetAddressOf()),
        "GpuParticleSystem failed to create depth sampler."
    );
}

void GpuParticleSystem::Simulate(
    GraphicsDevice &graphics,
    const Camera &camera,
    const float aspectRatio,
    const float deltaTimeSeconds,
    ID3D11ShaderResourceView *const sceneDepthShaderResourceView
)
{
    ID3D11DeviceContext *const context = graphics.GetImmediateContext();
    if (context == nullptr)
    {
        return;
    }

    const std::uint32_t spawnCount = ConsumeSpawnCount(deltaTimeSeconds);
    const std::uint32_t spawnStart = m_spawnCursor;
    if (m_maxParticles > 0u)
    {
        m_spawnCursor = (m_spawnCursor + spawnCount) & (m_maxParticles - 1u);
    }

    const Matrix view = camera.GetViewMatrix();
    const Matrix projection = camera.GetProjectionMatrix(aspectRatio);
    const Matrix viewProjection = view * projection;
    const Vector3 cameraWorldPosition = CameraWorldPositionFromViewMatrix(view);

    GpuParticleUpdateConstants constants{};
    constants.ViewProjection = viewProjection.Transpose();
    constants.InverseProjection = projection.Invert().Transpose();
    constants.InverseView = view.Invert().Transpose();
    constants.EmitterCenterAndDeltaTime = DirectX::XMFLOAT4(
        m_emitter.SpawnVolumeCenter.x,
        m_emitter.SpawnVolumeCenter.y,
        m_emitter.SpawnVolumeCenter.z,
        (std::max)(deltaTimeSeconds, 0.0f)
    );
    constants.EmitterExtentsAndLifetime = DirectX::XMFLOAT4(
        m_emitter.SpawnVolumeHalfExtents.x,
        m_emitter.SpawnVolumeHalfExtents.y,
        m_emitter.SpawnVolumeHalfExtents.z,
        (std::max)(m_emitter.LifetimeSeconds, 0.01f)
    );
    constants.VelocityMinAndStartSize = DirectX::XMFLOAT4(
        m_emitter.InitialVelocityMin.x,
        m_emitter.InitialVelocityMin.y,
        m_emitter.InitialVelocityMin.z,
        (std::max)(m_emitter.StartSize, 0.001f)
    );
    constants.VelocityMaxAndCollisionDamping = DirectX::XMFLOAT4(
        m_emitter.InitialVelocityMax.x,
        m_emitter.InitialVelocityMax.y,
        m_emitter.InitialVelocityMax.z,
        std::clamp(m_emitter.CollisionDamping, 0.0f, 1.0f)
    );
    constants.GravityAndCollisionBias = DirectX::XMFLOAT4(
        m_emitter.Gravity.x,
        m_emitter.Gravity.y,
        m_emitter.Gravity.z,
        (std::max)(m_emitter.CollisionDepthBias, 0.0f)
    );
    constants.ParticleColor = DirectX::XMFLOAT4(
        m_emitter.Color.x,
        m_emitter.Color.y,
        m_emitter.Color.z,
        m_emitter.Color.w
    );
    constants.CameraWorldPosition = DirectX::XMFLOAT4(
        cameraWorldPosition.x,
        cameraWorldPosition.y,
        cameraWorldPosition.z,
        1.0f
    );
    constants.DepthTextureSizeAndCollisionEnabled = DirectX::XMFLOAT4(
        graphics.GetWidth() > 0 ? 1.0f / static_cast<float>(graphics.GetWidth()) : 0.0f,
        graphics.GetHeight() > 0 ? 1.0f / static_cast<float>(graphics.GetHeight()) : 0.0f,
        sceneDepthShaderResourceView != nullptr && m_emitter.DepthCollisionEnabled ? 1.0f : 0.0f,
        0.0f
    );
    constants.MaxParticles = m_maxParticles;
    constants.SpawnStartIndex = spawnStart;
    constants.SpawnCount = spawnCount;
    constants.FrameIndex = ++m_frameIndex;

    UploadConstantBuffer(
        context,
        m_updateConstantBuffer.Get(),
        &constants,
        sizeof(GpuParticleUpdateConstants),
        "GpuParticleSystem failed to upload update constants."
    );

    ID3D11UnorderedAccessView *const unorderedAccessViews[] =
    {
        m_particleUnorderedAccessView.Get(),
        m_sortKeyUnorderedAccessView.Get()
    };
    ID3D11ShaderResourceView *const depthShaderResourceViews[] = {sceneDepthShaderResourceView};
    ID3D11SamplerState *const samplers[] = {m_depthSampler.Get()};
    ID3D11Buffer *const constantBuffers[] = {m_updateConstantBuffer.Get()};

    context->CSSetShader(m_updateComputeShader.Get(), nullptr, 0u);
    context->CSSetConstantBuffers(0, 1, constantBuffers);
    context->CSSetShaderResources(0, 1, depthShaderResourceViews);
    context->CSSetSamplers(0, 1, samplers);
    context->CSSetUnorderedAccessViews(0, 2, unorderedAccessViews, nullptr);
    context->Dispatch((m_maxParticles + kParticleThreadGroupSize - 1u) / kParticleThreadGroupSize, 1u, 1u);

    ID3D11UnorderedAccessView *const nullUnorderedAccessViews[2] = {};
    ID3D11ShaderResourceView *const nullShaderResourceViews[1] = {};
    ID3D11Buffer *const nullConstantBuffers[1] = {};
    context->CSSetUnorderedAccessViews(0, 2, nullUnorderedAccessViews, nullptr);
    context->CSSetShaderResources(0, 1, nullShaderResourceViews);
    context->CSSetConstantBuffers(0, 1, nullConstantBuffers);
    context->CSSetShader(nullptr, nullptr, 0u);

    Sort(context);
}

void GpuParticleSystem::Sort(ID3D11DeviceContext *const context)
{
    if (context == nullptr || m_maxParticles < 2u)
    {
        return;
    }

    ID3D11UnorderedAccessView *const unorderedAccessViews[] = {m_sortKeyUnorderedAccessView.Get()};
    ID3D11Buffer *const constantBuffers[] = {m_sortConstantBuffer.Get()};

    context->CSSetShader(m_sortComputeShader.Get(), nullptr, 0u);
    context->CSSetConstantBuffers(0, 1, constantBuffers);
    context->CSSetUnorderedAccessViews(0, 1, unorderedAccessViews, nullptr);

    for (std::uint32_t sortLevel = 2u; sortLevel <= m_maxParticles; sortLevel <<= 1u)
    {
        for (std::uint32_t compareDistance = sortLevel >> 1u; compareDistance > 0u; compareDistance >>= 1u)
        {
            GpuParticleSortConstants constants{};
            constants.SortLevel = sortLevel;
            constants.CompareDistance = compareDistance;
            constants.ElementCount = m_maxParticles;
            UploadConstantBuffer(
                context,
                m_sortConstantBuffer.Get(),
                &constants,
                sizeof(GpuParticleSortConstants),
                "GpuParticleSystem failed to upload sort constants."
            );

            context->Dispatch(
                (m_maxParticles + kParticleThreadGroupSize - 1u) / kParticleThreadGroupSize,
                1u,
                1u
            );
        }
    }

    ID3D11UnorderedAccessView *const nullUnorderedAccessViews[1] = {};
    ID3D11Buffer *const nullConstantBuffers[1] = {};
    context->CSSetUnorderedAccessViews(0, 1, nullUnorderedAccessViews, nullptr);
    context->CSSetConstantBuffers(0, 1, nullConstantBuffers);
    context->CSSetShader(nullptr, nullptr, 0u);
}

void GpuParticleSystem::Render(
    GraphicsDevice &graphics,
    const Camera &camera,
    const float aspectRatio,
    ID3D11ShaderResourceView *const sceneDepthShaderResourceView
)
{
    ID3D11DeviceContext *const context = graphics.GetImmediateContext();
    if (context == nullptr || m_commonStates == nullptr)
    {
        return;
    }

    const Matrix view = camera.GetViewMatrix();
    const Matrix projection = camera.GetProjectionMatrix(aspectRatio);
    const Matrix viewProjection = view * projection;
    const Matrix inverseView = view.Invert();

    Vector3 cameraRight(inverseView._11, inverseView._12, inverseView._13);
    Vector3 cameraUp(inverseView._21, inverseView._22, inverseView._23);
    cameraRight.Normalize();
    cameraUp.Normalize();

    GpuParticleDrawConstants constants{};
    constants.ViewProjection = viewProjection.Transpose();
    constants.CameraRightAndSizeScale = DirectX::XMFLOAT4(
        cameraRight.x,
        cameraRight.y,
        cameraRight.z,
        (std::max)(m_emitter.DrawSizeScale, 0.0f)
    );
    constants.CameraUpAndAlphaScale = DirectX::XMFLOAT4(
        cameraUp.x,
        cameraUp.y,
        cameraUp.z,
        std::clamp(m_emitter.DrawAlphaScale, 0.0f, 8.0f)
    );
    constants.ScreenInverseSizeAndDepthBias = DirectX::XMFLOAT4(
        graphics.GetWidth() > 0 ? 1.0f / static_cast<float>(graphics.GetWidth()) : 0.0f,
        graphics.GetHeight() > 0 ? 1.0f / static_cast<float>(graphics.GetHeight()) : 0.0f,
        m_emitter.CollisionDepthBias,
        sceneDepthShaderResourceView != nullptr ? 1.0f : 0.0f
    );

    UploadConstantBuffer(
        context,
        m_drawConstantBuffer.Get(),
        &constants,
        sizeof(GpuParticleDrawConstants),
        "GpuParticleSystem failed to upload draw constants."
    );

    ID3D11ShaderResourceView *const shaderResourceViews[] =
    {
        m_particleShaderResourceView.Get(),
        m_sortKeyShaderResourceView.Get(),
        sceneDepthShaderResourceView
    };
    ID3D11SamplerState *const samplers[] = {m_depthSampler.Get()};
    ID3D11Buffer *const constantBuffers[] = {m_drawConstantBuffer.Get()};

    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(m_drawVertexShader.Get(), nullptr, 0u);
    context->VSSetConstantBuffers(0, 1, constantBuffers);
    context->VSSetShaderResources(0, 2, shaderResourceViews);
    context->PSSetShader(m_drawPixelShader.Get(), nullptr, 0u);
    context->PSSetConstantBuffers(0, 1, constantBuffers);
    context->PSSetShaderResources(2, 1, &shaderResourceViews[2]);
    context->PSSetSamplers(0, 1, samplers);

    context->OMSetBlendState(m_commonStates->NonPremultiplied(), nullptr, 0xFFFFFFFFu);
    context->OMSetDepthStencilState(m_commonStates->DepthRead(), 0u);
    context->RSSetState(m_commonStates->CullNone());

    context->DrawInstanced(6u, m_maxParticles, 0u, 0u);

    ID3D11ShaderResourceView *const nullVertexShaderResources[2] = {};
    ID3D11ShaderResourceView *const nullPixelShaderResources[1] = {};
    ID3D11Buffer *const nullConstantBuffers[1] = {};
    context->VSSetShaderResources(0, 2, nullVertexShaderResources);
    context->PSSetShaderResources(2, 1, nullPixelShaderResources);
    context->VSSetConstantBuffers(0, 1, nullConstantBuffers);
    context->PSSetConstantBuffers(0, 1, nullConstantBuffers);
    context->OMSetBlendState(m_commonStates->Opaque(), nullptr, 0xFFFFFFFFu);
    context->OMSetDepthStencilState(m_commonStates->DepthDefault(), 0u);
    context->RSSetState(nullptr);
}

std::uint32_t GpuParticleSystem::ConsumeSpawnCount(const float deltaTimeSeconds) noexcept
{
    if (m_maxParticles == 0u || !m_emitter.Enabled || m_emitter.SpawnRateParticlesPerSecond <= 0.0f)
    {
        return 0u;
    }

    m_spawnAccumulator += (std::max)(deltaTimeSeconds, 0.0f) * m_emitter.SpawnRateParticlesPerSecond;
    const float spawnCountFloat = std::floor(m_spawnAccumulator);
    const auto spawnCount = static_cast<std::uint32_t>((std::max)(spawnCountFloat, 0.0f));
    m_spawnAccumulator -= static_cast<float>(spawnCount);
    return (std::min)(spawnCount, m_maxParticles);
}

std::uint32_t GpuParticleSystem::RoundDownToPowerOfTwo(std::uint32_t value) noexcept
{
    if (value == 0u)
    {
        return 0u;
    }

    std::uint32_t result = 1u;
    while ((result << 1u) != 0u && (result << 1u) <= value)
    {
        result <<= 1u;
    }
    return result;
}
