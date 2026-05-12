#ifndef PINGPONG_GBUFFERPICKINGSERVICE_H
#define PINGPONG_GBUFFERPICKINGSERVICE_H

#include <SimpleMath.h>

#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>

class Camera;
class GraphicsDevice;
class GBufferResources;

struct GBufferPickRequest final
{
    int ScreenX{0};
    int ScreenY{0};
};

struct GBufferPickResult final
{
    bool Hit{false};
    std::uint32_t ObjectId{0u};
    int ScreenX{0};
    int ScreenY{0};
    float Depth{0.0f};
    DirectX::SimpleMath::Vector3 WorldPosition{};
    DirectX::SimpleMath::Vector3 WorldNormal{};
};

class GBufferPickingService final
{
public:
    void Initialize(GraphicsDevice &graphics);

    void Shutdown() noexcept;

    [[nodiscard]] GBufferPickResult Pick(
        GraphicsDevice &graphics,
        const GBufferResources &gBuffer,
        const Camera &camera,
        float viewportAspectRatio,
        int screenX,
        int screenY
    );

private:
    void EnsureStagingTextures(ID3D11Device *device);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_stagingObjectId;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_stagingNormal;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_stagingDepth;

    bool m_initialized{false};
};

#endif
