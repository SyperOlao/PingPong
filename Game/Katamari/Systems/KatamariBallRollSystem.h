#ifndef PINGPONG_KATAMARIBALLROLLSYSTEM_H
#define PINGPONG_KATAMARIBALLROLLSYSTEM_H

#include "Core/Gameplay/ISceneSystem.h"

#include <SimpleMath.h>

struct KatamariWorldContext;

class KatamariBallRollSystem final : public ISceneSystem
{
public:
    explicit KatamariBallRollSystem(KatamariWorldContext *gameplayWorld) noexcept;

    void Initialize(Scene &scene, AppContext &context) override;

    void Update(Scene &scene, AppContext &context, float deltaTime) override;

private:
    KatamariWorldContext *GameplayWorld{nullptr};
    DirectX::SimpleMath::Quaternion RollOrientation{0.0f, 0.0f, 0.0f, 1.0f};
    DirectX::SimpleMath::Vector3 PreviousBallPosition{0.0f, 0.0f, 0.0f};
    DirectX::SimpleMath::Vector3 SmoothedRollDirection{0.0f, 0.0f, 1.0f};
    bool HasPreviousBallPosition{false};
    bool HasSmoothedRollDirection{false};
};

#endif
