//
// Created by SyperOlao on 18.03.2026.
//

#ifndef MYPROJECT_IGAME_H
#define MYPROJECT_IGAME_H

struct AppContext;

class IGame {
public:
    virtual ~IGame() = default;

    virtual void Initialize(AppContext &context) = 0;

    virtual void Update(AppContext &context, float deltaTime) = 0;

    virtual void Render(AppContext &context) = 0;

    virtual void Shutdown(AppContext &context) {
        (void) context;
    }
};

#endif //MYPROJECT_IGAME_H
