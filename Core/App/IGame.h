//
// Created by SyperOlao on 18.03.2026.
//

#ifndef PINGPONG_IGAME_H
#define PINGPONG_IGAME_H

struct AppContext;

class IGame {
public:
    virtual ~IGame() = default;

    virtual void Initialize(AppContext &context) = 0;

    virtual void Update(AppContext &context, float deltaTime) = 0;

    virtual void Render(AppContext &context) = 0;

    virtual void Shutdown(AppContext &context) {
    }
};

#endif //PINGPONG_IGAME_H
