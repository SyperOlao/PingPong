//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_GAME_H
#define PINGPONG_GAME_H


class Renderer;

class Game {
public:
    void Initialize(int windowWidth, int windowHeight);
    void Update(float deltaTime);
    void Render(const Renderer& renderer) const;

private:
    int m_windowWidth {0};
    int m_windowHeight {0};
};


#endif //PINGPONG_GAME_H
