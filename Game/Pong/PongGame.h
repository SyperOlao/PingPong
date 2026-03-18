//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_GAME_H
#define PINGPONG_GAME_H
#include "Core/App/IGame.h"



class PongGame : public IGame
{
public:
    void Initialize(AppContext& context) override;
    void Update(AppContext& context, float deltaTime) override;

    void Render(AppContext& context) override;
};

#endif //PINGPONG_GAME_H
