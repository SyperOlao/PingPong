#ifndef PINGPONG_COLLISIONSYSTEM_H
#define PINGPONG_COLLISIONSYSTEM_H

#include "Core/Gameplay/ISceneSystem.h"
#include "Core/Physics/UniformSpatialGrid3D.h"

#include <cstdint>
#include <vector>

class CollisionSystem final : public ISceneSystem
{
public:
    void Initialize(Scene &scene, AppContext &context) override;

    void Update(Scene &scene, AppContext &context, float deltaTime) override;

private:
    UniformSpatialGrid3D m_spatialGrid{};
    std::vector<std::uint32_t> m_queryScratch{};
};

#endif
