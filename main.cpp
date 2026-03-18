#include "Core/App/Application.h"
#include <iostream>

#include <Windows.h>
#include <exception>

int main()
{
    try
    {
        std::cout << "[LOG] main started\n";

        HINSTANCE hInstance = GetModuleHandleW(nullptr);
        std::cout << "[LOG] got module handle\n";

        Application app(hInstance);
        std::cout << "[LOG] Application constructed\n";

        const int result = app.Run();
        std::cout << "[LOG] app.Run() finished with code: " << result << "\n";

        return result;
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