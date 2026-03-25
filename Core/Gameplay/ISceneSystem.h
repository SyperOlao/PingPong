#ifndef PINGPONG_ISCENESYSTEM_H
#define PINGPONG_ISCENESYSTEM_H

class AppContext;
class Scene;

class ISceneSystem
{
public:
    virtual ~ISceneSystem() = default;

    virtual void Initialize(Scene &scene, AppContext &context) = 0;

    virtual void Update(Scene &scene, AppContext &context, float deltaTime) = 0;

    virtual void Render(Scene &scene, AppContext &context);
};

#endif
