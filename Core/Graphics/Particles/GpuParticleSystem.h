#ifndef PINGPONG_GPUPARTICLESYSTEM_H
#define PINGPONG_GPUPARTICLESYSTEM_H

#include "Core/Graphics/Particles/GpuParticleTypes.h"

#include <d3d11.h>
#include <directxtk/CommonStates.h>
#include <cstdint>
#include <memory>
#include <wrl/client.h>

class Camera;
class GraphicsDevice;

class GpuParticleSystem final
{
public:
    GpuParticleSystem() = default;
    ~GpuParticleSystem();

    GpuParticleSystem(const GpuParticleSystem &) = delete;
    GpuParticleSystem &operator=(const GpuParticleSystem &) = delete;
    GpuParticleSystem(GpuParticleSystem &&) = delete;
    GpuParticleSystem &operator=(GpuParticleSystem &&) = delete;

    void Initialize(GraphicsDevice &graphics, std::uint32_t maxParticles = 4096u);
    void Shutdown() noexcept;

    void SetEmitterDesc(const GpuParticleEmitterDesc &desc) noexcept;
    [[nodiscard]] const GpuParticleEmitterDesc &GetEmitterDesc() const noexcept;
    [[nodiscard]] GpuParticleEmitterDesc &GetMutableEmitterDesc() noexcept;

    [[nodiscard]] std::uint32_t GetMaxParticles() const noexcept;
    [[nodiscard]] bool IsInitialized() const noexcept;

    void SimulateAndRender(
        GraphicsDevice &graphics,
        const Camera &camera,
        float aspectRatio,
        float deltaTimeSeconds,
        ID3D11ShaderResourceView *sceneDepthShaderResourceView
    );

private:
    void CreateShaders(ID3D11Device *device);
    void CreateBuffers(ID3D11Device *device);
    void CreateConstantBuffers(ID3D11Device *device);
    void CreateViews(ID3D11Device *device);
    void CreateSampler(ID3D11Device *device);

    void Simulate(
        GraphicsDevice &graphics,
        const Camera &camera,
        float aspectRatio,
        float deltaTimeSeconds,
        ID3D11ShaderResourceView *sceneDepthShaderResourceView
    );
    void Sort(ID3D11DeviceContext *context);
    void Render(
        GraphicsDevice &graphics,
        const Camera &camera,
        float aspectRatio,
        ID3D11ShaderResourceView *sceneDepthShaderResourceView
    );

    [[nodiscard]] std::uint32_t ConsumeSpawnCount(float deltaTimeSeconds) noexcept;
    [[nodiscard]] static std::uint32_t RoundDownToPowerOfTwo(std::uint32_t value) noexcept;

private:
    std::uint32_t m_maxParticles{0u};
    std::uint32_t m_spawnCursor{0u};
    std::uint32_t m_frameIndex{0u};
    float m_spawnAccumulator{0.0f};
    GpuParticleEmitterDesc m_emitter{};

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_particleBuffer{};
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_sortKeyBuffer{};
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_particleShaderResourceView{};
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_sortKeyShaderResourceView{};
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_particleUnorderedAccessView{};
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_sortKeyUnorderedAccessView{};

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_updateConstantBuffer{};
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_sortConstantBuffer{};
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_drawConstantBuffer{};

    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_updateComputeShader{};
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_sortComputeShader{};
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_drawVertexShader{};
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_drawPixelShader{};
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_depthSampler{};
    std::unique_ptr<DirectX::CommonStates> m_commonStates{};
};

#endif
