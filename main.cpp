#include "Core/App/Application.h"
#include <iostream>

#include <Windows.h>
#include <exception>

int main()
{
    try
    {
        HINSTANCE hInstance = GetModuleHandleW(nullptr);
        Application app(hInstance);
        return app.Run();
    }
    catch (const std::exception& exception)
    {
        MessageBoxA(nullptr, exception.what(), "Fatal Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}