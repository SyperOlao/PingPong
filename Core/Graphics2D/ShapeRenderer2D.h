//
// Created by SyperOlao on 18.03.2026.
//

#ifndef MYPROJECT_SHAPERENDERER2D_H
#define MYPROJECT_SHAPERENDERER2D_H
#include <d3d11.h>
#include <wrl/client.h>

#include "../Graphics/Color.h"
#include "Core/UI/Common/RectF.h"

class GraphicsDevice;


class ShapeRenderer2D final
{
public:
    ShapeRenderer2D() = default;
    ~ShapeRenderer2D() = default;

    ShapeRenderer2D(const ShapeRenderer2D&) = delete;
    ShapeRenderer2D& operator=(const ShapeRenderer2D&) = delete;

    ShapeRenderer2D(ShapeRenderer2D&&) = delete;
    ShapeRenderer2D& operator=(ShapeRenderer2D&&) = delete;

    void Initialize(GraphicsDevice& graphics);

    void DrawFilledRect(const RectF& rect, const Color& color) const;
    void DrawFilledRect(float x, float y, float width, float height, const Color& color) const;

    [[nodiscard]] bool IsInitialized() const noexcept;

private:
    struct Vertex2D final
    {
        float x;
        float y;
        float z;

        float r;
        float g;
        float b;
        float a;
    };

private:
    void CreateShaders();
    void CreateDynamicVertexBuffer();
    void BindPipeline() const;

    [[nodiscard]] float ToNdcX(float x) const noexcept;
    [[nodiscard]] float ToNdcY(float y) const noexcept;

private:
    GraphicsDevice* m_graphics{nullptr};

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
};

#endif //MYPROJECT_SHAPERENDERER2D_H