#ifndef PINGPONG_UINTRENDERTARGET2D_H
#define PINGPONG_UINTRENDERTARGET2D_H

#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>

class GraphicsDevice;

class UIntRenderTarget2D final
{
public:
    [[nodiscard]] bool IsCreated() const noexcept;

    [[nodiscard]] std::uint32_t GetWidth() const noexcept;

    [[nodiscard]] std::uint32_t GetHeight() const noexcept;

    [[nodiscard]] DXGI_FORMAT GetFormat() const noexcept;

    void Create(ID3D11Device *device, std::uint32_t width, std::uint32_t height, DXGI_FORMAT format);

    void Destroy() noexcept;

    void Resize(ID3D11Device *device, std::uint32_t width, std::uint32_t height);

    [[nodiscard]] ID3D11Texture2D *GetTexture() const noexcept;

    [[nodiscard]] ID3D11RenderTargetView *GetRenderTargetView() const noexcept;

    [[nodiscard]] ID3D11ShaderResourceView *GetShaderResourceView() const noexcept;

    void Clear(GraphicsDevice &graphics, std::uint32_t clearValue) const;

private:
    void CreateInternal(ID3D11Device *device, std::uint32_t width, std::uint32_t height);

    std::uint32_t m_width{0};

    std::uint32_t m_height{0};

    DXGI_FORMAT m_format{DXGI_FORMAT_UNKNOWN};

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
};

#endif
