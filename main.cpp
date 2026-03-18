#include "Core/App/Application.h"
#include <iostream>

#include <Windows.h>
#include <exception>
#include "Core/App/Application.h"
#include "Game/Pong/PongGame.h"

int main()
{
    try
    {
        HINSTANCE__ *const hInstance = GetModuleHandleW(nullptr);

        Application app(
            hInstance,
            std::make_unique<PongGame>(),
            ApplicationDesc{
                .Title = L"Pong DX11",
                .ClientWidth = 1280,
                .ClientHeight = 720,
                .VSync = true,
                .ClearColor = Color(0.05f, 0.05f, 0.08f, 1.0f)
            }
        );

        return app.Run();
    }
    catch (const std::exception& exception)
    {
        const std::string message = std::string("Fatal error:\n") + exception.what();

        std::cerr << message << std::endl;
        OutputDebugStringA(message.c_str());
        MessageBoxA(nullptr, message.c_str(), "Fatal Error", MB_OK | MB_ICONERROR);

        return -1;
    }
    catch (...)
    {
        const char* message = "Unknown fatal error";

        std::cerr << message << std::endl;
        OutputDebugStringA(message);
        MessageBoxA(nullptr, message, "Fatal Error", MB_OK | MB_ICONERROR);

        return -1;
    }
}