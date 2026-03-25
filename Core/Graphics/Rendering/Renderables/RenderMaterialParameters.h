#ifndef PINGPONG_RENDERMATERIALPARAMETERS_H
#define PINGPONG_RENDERMATERIALPARAMETERS_H

#include <SimpleMath.h>

struct RenderMaterialParameters final
{
    DirectX::SimpleMath::Color BaseColor{1.0f, 1.0f, 1.0f, 1.0f};
    bool Wireframe{false};
    float AmbientFactor{0.08f};
    float SpecularPower{48.0f};
    DirectX::SimpleMath::Color SpecularColor{1.0f, 1.0f, 1.0f, 1.0f};
    DirectX::SimpleMath::Color EmissiveColor{0.0f, 0.0f, 0.0f, 1.0f};
};

#endif
