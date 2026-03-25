#ifndef PINGPONG_DEPTHRENDERTARGET2D_H
#define PINGPONG_DEPTHRENDERTARGET2D_H

#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>

class DepthRenderTarget2D final
{
public:
    [[nodiscard]] bool IsCreated() const noexcept;

    [[nodiscard]] std::uint32_t GetWidth() const noexcept;

    [[nodiscard]] std::uint32_t GetHeight() const noexcept;

    void Create(ID3D11Device *device, std::uint32_t width, std::uint32_t height);

    void Destroy() noexcept;

    void Resize(ID3D11Device *device, std::uint32_t width, std::uint32_t height);

    [[nodiscard]] ID3D11Texture2D *GetTexture() const noexcept;

    [[nodiscard]] ID3D11DepthStencilView *GetDepthStencilView() const noexcept;

    [[nodiscard]] ID3D11ShaderResourceView *GetShaderResourceView() const noexcept;

private:
    void CreateInternal(ID3D11Device *device, std::uint32_t width, std::uint32_t height);

    std::uint32_t m_width{0};

    std::uint32_t m_height{0};

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;

    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
};

#endif
