//
// Created by SyperOlao on 19.03.2026.
//

#ifndef PINGPONG_PRIMITIVERENDERER3D_H
#define PINGPONG_PRIMITIVERENDERER3D_H

#include <SimpleMath.h>
#include <d3d11.h>
#include <directxtk/GeometricPrimitive.h>
#include <memory>
#include <wrl/client.h>

class GraphicsDevice;

class PrimitiveRenderer3D final
{
public:
    void Initialize(GraphicsDevice& graphics);
    ~PrimitiveRenderer3D();
    void BeginFrame3D() const;

    void DrawBox(
       const DirectX::SimpleMath::Matrix& world,
       const DirectX::SimpleMath::Matrix& view,
       const DirectX::SimpleMath::Matrix& projection,
       const DirectX::SimpleMath::Color& color
   ) const;

    void DrawSphere(
        const DirectX::SimpleMath::Matrix& world,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Color& color
    ) const;

private:
    void CreatePrimitives();
    void EnsureDepthResources() const;
    void BindTargets() const;

private:
    GraphicsDevice* m_graphics{nullptr};

    std::unique_ptr<DirectX::DX11::GeometricPrimitive> m_box;
    std::unique_ptr<DirectX::DX11::GeometricPrimitive> m_sphere;

    mutable Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthTexture;
    mutable Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;

    mutable int m_cachedDepthWidth{0};
    mutable int m_cachedDepthHeight{0};
};

#endif //PINGPONG_PRIMITIVERENDERER3D_H
