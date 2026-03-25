#ifndef PINGPONG_PRIMITIVERENDERER3D_H
#define PINGPONG_PRIMITIVERENDERER3D_H

#include <vector>

#include <SimpleMath.h>
#include <d3d11.h>
#include <directxtk/GeometricPrimitive.h>
#include <memory>
#include <wrl/client.h>

#include "Core/Graphics/Rendering/Lighting/SceneLighting3D.h"
#include "Core/Graphics/Rendering/Renderables/RenderMaterialParameters.h"
#include "Core/Graphics/Vertex3D.h"

class GraphicsDevice;

class PrimitiveRenderer3D final
{
public:
    void Initialize(GraphicsDevice &graphics);

    ~PrimitiveRenderer3D();

    void DrawBox(
        const DirectX::SimpleMath::Matrix &world,
        const DirectX::SimpleMath::Matrix &view,
        const DirectX::SimpleMath::Matrix &projection,
        const DirectX::SimpleMath::Color &color
    ) const;

    void DrawSphere(
        const DirectX::SimpleMath::Matrix &world,
        const DirectX::SimpleMath::Matrix &view,
        const DirectX::SimpleMath::Matrix &projection,
        const DirectX::SimpleMath::Color &color
    ) const;

    void DrawBoxLit(
        const DirectX::SimpleMath::Matrix &world,
        const DirectX::SimpleMath::Matrix &view,
        const DirectX::SimpleMath::Matrix &projection,
        const DirectX::SimpleMath::Vector3 &cameraWorldPosition,
        const SceneLightingDescriptor3D &lighting,
        const RenderMaterialParameters &material
    ) const;

    void DrawSphereLit(
        const DirectX::SimpleMath::Matrix &world,
        const DirectX::SimpleMath::Matrix &view,
        const DirectX::SimpleMath::Matrix &projection,
        const DirectX::SimpleMath::Vector3 &cameraWorldPosition,
        const SceneLightingDescriptor3D &lighting,
        const RenderMaterialParameters &material
    ) const;

    void DrawOrbit(
        const std::vector<DirectX::SimpleMath::Vector3> &points,
        const DirectX::SimpleMath::Matrix &view,
        const DirectX::SimpleMath::Matrix &projection,
        const DirectX::SimpleMath::Color &color
    ) const;

    void DrawLineList(
        const Vertex3D *vertices,
        UINT vertexCount,
        const DirectX::SimpleMath::Matrix &view,
        const DirectX::SimpleMath::Matrix &projection
    ) const;

private:
    void CreatePrimitives();

    void CreateLineResources();

    void CreateLitResources();

    void BindTargets() const;

    void DrawLitMesh(
        ID3D11Buffer *vertexBuffer,
        ID3D11Buffer *indexBuffer,
        UINT indexCount,
        const DirectX::SimpleMath::Matrix &world,
        const DirectX::SimpleMath::Matrix &view,
        const DirectX::SimpleMath::Matrix &projection,
        const DirectX::SimpleMath::Vector3 &cameraWorldPosition,
        const SceneLightingDescriptor3D &lighting,
        const RenderMaterialParameters &material
    ) const;

private:
    GraphicsDevice *m_graphics{nullptr};

    std::unique_ptr<DirectX::DX11::GeometricPrimitive> m_box;
    std::unique_ptr<DirectX::DX11::GeometricPrimitive> m_sphere;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_lineVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_linePixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_lineInputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_lineVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_lineConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_debugLineVertexBuffer;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_litVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_litPixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_litInputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_litBoxVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_litBoxIndexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_litSphereVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_litSphereIndexBuffer;
    UINT m_litBoxIndexCount{0u};
    UINT m_litSphereIndexCount{0u};
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_cameraConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_objectConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_materialConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightsConstantBuffer;
};

#endif
