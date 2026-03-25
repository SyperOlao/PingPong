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

    void DrawOrbit(
        const std::vector<DirectX::SimpleMath::Vector3>& points,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Color& color
    ) const;

private:
    void CreatePrimitives();
    void CreateLineResources();
    void BindTargets() const;

private:
    GraphicsDevice* m_graphics{nullptr};

    std::unique_ptr<DirectX::DX11::GeometricPrimitive> m_box;
    std::unique_ptr<DirectX::DX11::GeometricPrimitive> m_sphere;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_lineVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_linePixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_lineInputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_lineVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_lineConstantBuffer;
};
#endif //PINGPONG_PRIMITIVERENDERER3D_H
