#ifndef PINGPONG_GAMEPLAYCOMPONENTS_H
#define PINGPONG_GAMEPLAYCOMPONENTS_H

#include "Core/Gameplay/EntityId.h"
#include "Core/Math/Transform3D.h"

#include <SimpleMath.h>

#include <cstdint>
#include <memory>
#include <string>

class ModelAsset;

struct TransformComponent final
{
    Transform3D Local{};
    DirectX::SimpleMath::Matrix WorldMatrix{DirectX::SimpleMath::Matrix::Identity};
};

struct ModelComponent final
{
    std::shared_ptr<ModelAsset> Asset{};
    bool Visible{true};
    DirectX::SimpleMath::Color Tint{1.0f, 1.0f, 1.0f, 1.0f};
    bool DebugDrawBounds{false};
};

struct SphereColliderComponent final
{
    DirectX::SimpleMath::Vector3 LocalCenter{0.0f, 0.0f, 0.0f};
    float Radius{1.0f};
    std::uint32_t CollisionLayer{0u};
    std::uint32_t CollidesWithMask{0xFFFFFFFFu};
    bool IsTrigger{false};
};

struct TagComponent final
{
    std::string TagText{};
    std::uint32_t TagId{0u};
};

struct VelocityComponent final
{
    DirectX::SimpleMath::Vector3 LinearVelocity{0.0f, 0.0f, 0.0f};
};

struct AttachmentComponent final
{
    EntityId ParentEntityId{0u};
    DirectX::SimpleMath::Vector3 LocalOffset{0.0f, 0.0f, 0.0f};
};

#endif
