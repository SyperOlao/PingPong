//
// Created by SyperOlao on 17.03.2026.
//

#include "Game.h"
#include "../Core/Render/Color.h"
#include "../Core/Render/Renderer.h"

void Game::Initialize(const int windowWidth, const int windowHeight) {
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;
}

void Game::Update(float deltaTime) {
}

void Game::Render(const Renderer &renderer) const {
    constexpr Color white {1.0f, 1.0f, 1.0f, 1.0f};

    renderer.DrawRectangle(40.0f, 250.0f, 20.0f, 120.0f, white);
    renderer.DrawRectangle(static_cast<float>(m_windowWidth - 60), 250.0f, 20.0f, 120.0f, white);
    renderer.DrawRectangle((static_cast<float>(m_windowWidth) / 2 - 5), 330.0f, 10.0f, 10.0f, white);
}
