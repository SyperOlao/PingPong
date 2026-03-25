#ifndef PINGPONG_GRAPHICSDEVICE_H
#define PINGPONG_GRAPHICSDEVICE_H

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "Color.h"

class GraphicsDevice final
{
public:
    GraphicsDevice() = default;

    ~GraphicsDevice() = default;

    GraphicsDevice(const GraphicsDevice &) = delete;

    GraphicsDevice &operator=(const GraphicsDevice &) = delete;

    GraphicsDevice(GraphicsDevice &&) = delete;

    GraphicsDevice &operator=(GraphicsDevice &&) = delete;

    void Initialize(HWND windowHandle, int width, int height);

    void Resize(int width, int height);

    void BeginFrame(const Color &clearColor);

    void EndFrame(bool vSync = true) const;

    void BindMainRenderTargets() const;

    void BindSingleRenderTarget(ID3D11RenderTargetView *renderTargetView) const;

    void BindRenderTargetsAndDepth(
        ID3D11RenderTargetView *const *renderTargetViews,
        UINT renderTargetCount,
        ID3D11DepthStencilView *depthStencilView
    ) const;

    void ClearRenderTargetView(ID3D11RenderTargetView *renderTargetView, const Color &clearColor) const;

    void ClearMainColor(const Color &clearColor) const;

    void ClearMainDepthStencil(float depthClearValue = 1.0f, uint8_t stencilClear = 0) const;

    void SetMainViewport() const;

    void BindDepthOnlyRenderTarget(ID3D11DepthStencilView *depthStencilView) const;

    void SetViewportPixels(float width, float height, float topLeftX = 0.0f, float topLeftY = 0.0f) const;

    void ClearDepthStencilView(ID3D11DepthStencilView *depthStencilView, float depthClearValue, uint8_t stencilClear = 0) const;

    [[nodiscard]] int GetWidth() const noexcept;

    [[nodiscard]] int GetHeight() const noexcept;

    [[nodiscard]] HWND GetWindowHandle() const noexcept;

    [[nodiscard]] ID3D11Device *GetDevice() const noexcept;

    [[nodiscard]] ID3D11DeviceContext *GetImmediateContext() const noexcept;

    [[nodiscard]] D3D_FEATURE_LEVEL GetFeatureLevel() const noexcept;

    [[nodiscard]] IDXGISwapChain *GetSwapChain() const noexcept;

    [[nodiscard]] ID3D11RenderTargetView *GetMainRenderTargetView() const noexcept;

    [[nodiscard]] ID3D11DepthStencilView *GetMainDepthStencilView() const noexcept;

private:
    void CreateDeviceAndSwapChain();

    void CreateRenderTarget();

    void CreateDepthStencil();

    void CreateMainPassDepthStencilState();

    void ReleaseRenderTarget() noexcept;

    void ReleaseDepthStencil() noexcept;

private:
    HWND m_windowHandle{nullptr};

    int m_width{0};

    int m_height{0};

    D3D_FEATURE_LEVEL m_featureLevel{D3D_FEATURE_LEVEL_11_0};

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;

    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;

    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_mainRenderTargetView;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_mainDepthTexture;

    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_mainDepthStencilView;

    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_mainPassDepthStencilState;
};

#endif
