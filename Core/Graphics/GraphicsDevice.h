//
// Created by SyperOlao on 18.03.2026.
//

#ifndef MYPROJECT_GRAPHICSDEVICE_H
#define MYPROJECT_GRAPHICSDEVICE_H
#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "Color.h"

class GraphicsDevice final
{
public:
    GraphicsDevice() = default;
    ~GraphicsDevice() = default;

    GraphicsDevice(const GraphicsDevice&) = delete;
    GraphicsDevice& operator=(const GraphicsDevice&) = delete;

    GraphicsDevice(GraphicsDevice&&) = delete;
    GraphicsDevice& operator=(GraphicsDevice&&) = delete;

    void Initialize(HWND windowHandle, int width, int height);
    void Resize(int width, int height);

    void BeginFrame(const Color& clearColor);
    void EndFrame(bool vSync = true);

    [[nodiscard]] int GetWidth() const noexcept;
    [[nodiscard]] int GetHeight() const noexcept;
    [[nodiscard]] HWND GetWindowHandle() const noexcept;

    [[nodiscard]] ID3D11Device* GetDevice() const noexcept;
    [[nodiscard]] ID3D11DeviceContext* GetImmediateContext() const noexcept;
    [[nodiscard]] IDXGISwapChain* GetSwapChain() const noexcept;
    [[nodiscard]] ID3D11RenderTargetView* GetRenderTargetView() const noexcept;

private:
    void CreateDeviceAndSwapChain();
    void CreateRenderTarget();
    void CreateViewport() const;
    void ReleaseRenderTarget() noexcept;

private:
    HWND m_windowHandle{nullptr};

    int m_width{0};
    int m_height{0};

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
};


#endif //MYPROJECT_GRAPHICSDEVICE_H