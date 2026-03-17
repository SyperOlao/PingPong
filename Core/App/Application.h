//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_APPLICATION_H
#define PINGPONG_APPLICATION_H
#include "Timer.h"
#include "../Input/InputSystem.h"
#include "../Render/Renderer.h"
#include "../Display/Window.h"
#include "../../Game/Game.h"
#include <windows.h>

class Application {
public:
    explicit Application(HINSTANCE hInstance);
    int Run();

private:
    void Initialize();
    void Update();
    void Render() const;


    HINSTANCE m_hInstance {nullptr};
    Window m_window;
    Renderer m_renderer;
    InputSystem m_input;
    Game m_game;
    Timer m_timer;

    bool m_isRunning {true};
};


#endif //PINGPONG_APPLICATION_H