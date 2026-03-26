#ifndef PINGPONG_COLLISIONDEBUGDRAW_H
#define PINGPONG_COLLISIONDEBUGDRAW_H

#include <SimpleMath.h>

class DebugDrawQueue;
class Scene;

class CollisionDebugDraw final
{
public:
    static void DrawBoxColliderWorldBounds(
        Scene &scene,
        DebugDrawQueue &queue,
        const DirectX::SimpleMath::Color &color
    );

    static void DrawContactNormalsFromLastFrame(
        Scene &scene,
        DebugDrawQueue &queue,
        const DirectX::SimpleMath::Color &color,
        float normalLength
    );

    static void DrawSphereColliderWorldBounds(
        Scene &scene,
        DebugDrawQueue &queue,
        const DirectX::SimpleMath::Color &color
    );

    static void DrawCollisionWorldBounds(
        Scene &scene,
        DebugDrawQueue &queue,
        const DirectX::SimpleMath::Color &color
    );
};

#endif
