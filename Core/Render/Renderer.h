//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_RENDERER_H
#define PINGPONG_RENDERER_H
#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <string_view>
#include "../Graphics/Color.h"

class Renderer {
public:
    Renderer() = default;
    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;
    Renderer(Renderer &&) = delete;
    Renderer &operator=(Renderer &&) = delete;
    void Initialize(HWND hWnd, int width, int height);
    void BeginFrame(const Color &clearColor) const;
    void EndFrame() const;
    void DrawRectangle(float x, float y, float width, float height, const Color &color) const;
    [[nodiscard]] int GetWidth() const noexcept;
    [[nodiscard]] int GetHeight() const noexcept;

private:
    void CreateDeviceAndSwapChain(HWND hWnd);
    void CreateRenderTarget();
    void CreateViewport() const;
    void LoadShaders();
    void CreateDynamicVertexBuffer();
    void SetupPipeline() const;

    [[nodiscard]] float ToNdcX(float x) const noexcept;
    [[nodiscard]] float ToNdcY(float y) const noexcept;

    static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
        std::wstring_view filePath,
        std::string_view entryPoint,
        std::string_view profile
    );

private:
    int m_width{0};
    int m_height{0};

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
};


#endif //PINGPONG_RENDERER_H
