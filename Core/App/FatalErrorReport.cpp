#include "Core/App/FatalErrorReport.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace
{
    [[nodiscard]] std::string BuildTimestampLocal()
    {
        const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        const std::time_t timeValue = std::chrono::system_clock::to_time_t(now);
        std::tm localParts{};
        if (localtime_s(&localParts, &timeValue) != 0)
        {
            return "unknown time";
        }
        std::ostringstream stream;
        stream << std::put_time(&localParts, "%Y-%m-%d %H:%M:%S");
        return stream.str();
    }

    [[nodiscard]] bool TryWriteLogFile(const std::filesystem::path &logPath, const std::string &bodyUtf8)
    {
        std::ofstream file(logPath, std::ios::binary | std::ios::trunc);
        if (!file.is_open())
        {
            return false;
        }

        constexpr unsigned char utf8Bom[] = {0xEFu, 0xBBu, 0xBFu};
        file.write(reinterpret_cast<const char *>(utf8Bom), static_cast<std::streamsize>(sizeof(utf8Bom)));
        file.write(bodyUtf8.data(), static_cast<std::streamsize>(bodyUtf8.size()));
        file.flush();
        return file.good();
    }

    [[nodiscard]] std::filesystem::path PrimaryLogPathNextToExecutable()
    {
        wchar_t modulePathWide[MAX_PATH]{};
        const DWORD moduleLength = GetModuleFileNameW(nullptr, modulePathWide, MAX_PATH);
        if (moduleLength == 0u || moduleLength >= MAX_PATH)
        {
            return {};
        }

        return std::filesystem::path(modulePathWide).parent_path() / L"MiniEngineDemo_LastError.txt";
    }

    [[nodiscard]] std::filesystem::path FallbackLogPathInTemp()
    {
        wchar_t tempWide[MAX_PATH]{};
        const DWORD tempLength = GetTempPathW(static_cast<DWORD>(std::size(tempWide)), tempWide);
        if (tempLength == 0u || tempLength >= std::size(tempWide))
        {
            return std::filesystem::path(L"MiniEngineDemo_LastError.txt");
        }

        return std::filesystem::path(tempWide) / L"MiniEngineDemo_LastError.txt";
    }
}

void FatalErrorReport::Report(const std::string &message)
{
    const std::string timestamp = BuildTimestampLocal();
    const std::string body =
        std::string("Timestamp (local): ") + timestamp + "\r\n\r\n" + message + "\r\n";

    std::filesystem::path logPath = PrimaryLogPathNextToExecutable();
    if (logPath.empty() || !TryWriteLogFile(logPath, body))
    {
        logPath = FallbackLogPathInTemp();
        if (!TryWriteLogFile(logPath, body))
        {
            logPath = std::filesystem::path(L"MiniEngineDemo_LastError.txt");
            TryWriteLogFile(logPath, body);
        }
    }

    const std::wstring dialogText =
        L"The application stopped due to an error.\r\n\r\n"
        L"Full details were written to this file (you can open it in Notepad):\r\n"
        + logPath.wstring();

    MessageBoxW(nullptr, dialogText.c_str(), L"Fatal Error", MB_OK | MB_ICONERROR | MB_TOPMOST);

    OutputDebugStringA(message.c_str());
    OutputDebugStringA("\n");
}
