#include "Core/App/Application.h"
#include <iostream>

#include <Windows.h>
#include <exception>
#include "Game/Pong/PongGame.h"
#include "Game/SolarSystem/SolarSystemGame.h"

enum class DemoType
{
    Pong,
    SolarSystem
};

int main()
{
    try
    {
        HINSTANCE__ *const hInstance = GetModuleHandleW(nullptr);

        constexpr auto demo = DemoType::Pong;

        std::unique_ptr<IGame> game;

        switch (demo)
        {
            case DemoType::Pong:
                game = std::make_unique<PongGame>();
                break;

            case DemoType::SolarSystem:
                game = std::make_unique<SolarSystemGame>();
                break;
        }

        Application app(hInstance, std::move(game));
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