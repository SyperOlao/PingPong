#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Core/Assets/AssetPathResolver.h"

#include <Windows.h>

#include <cwctype>
#include <stdexcept>
#include <string>

namespace {
    [[nodiscard]] std::wstring ToLowerInvariant(std::wstring value)
    {
        for (wchar_t &character : value)
        {
            character = static_cast<wchar_t>(std::towlower(static_cast<wint_t>(character)));
        }
        return value;
    }
}

std::filesystem::path AssetPathResolver::GetExecutableDirectory()
{
    std::wstring buffer(MAX_PATH, L'\0');

    DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0)
    {
        throw std::runtime_error("GetModuleFileNameW failed while resolving executable directory.");
    }

    while (length == buffer.size())
    {
        buffer.resize(buffer.size() * 2);

        length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0)
        {
            throw std::runtime_error("GetModuleFileNameW failed while resolving executable directory.");
        }
    }

    buffer.resize(length);
    return std::filesystem::path(buffer).parent_path();
}

std::filesystem::path AssetPathResolver::Resolve(const std::filesystem::path &contentRelativeOrAbsolutePath)
{
    if (contentRelativeOrAbsolutePath.empty())
    {
        throw std::invalid_argument("AssetPathResolver::Resolve received an empty path.");
    }

    if (contentRelativeOrAbsolutePath.is_absolute() && std::filesystem::exists(contentRelativeOrAbsolutePath))
    {
        return std::filesystem::weakly_canonical(contentRelativeOrAbsolutePath);
    }

    if (std::filesystem::exists(contentRelativeOrAbsolutePath))
    {
        return std::filesystem::weakly_canonical(contentRelativeOrAbsolutePath);
    }

    std::filesystem::path current = GetExecutableDirectory();

    for (int stepIndex = 0; stepIndex < 8; ++stepIndex)
    {
        const std::filesystem::path candidate = current / contentRelativeOrAbsolutePath;
        if (std::filesystem::exists(candidate))
        {
            return std::filesystem::weakly_canonical(candidate);
        }

        if (!current.has_parent_path())
        {
            break;
        }

        const std::filesystem::path parent = current.parent_path();
        if (parent == current)
        {
            break;
        }

        current = parent;
    }

    throw std::runtime_error("Asset file not found: " + contentRelativeOrAbsolutePath.string());
}

std::wstring AssetPathResolver::MakeCacheKey(const std::filesystem::path &resolvedAbsolutePath)
{
    if (resolvedAbsolutePath.empty())
    {
        throw std::invalid_argument("AssetPathResolver::MakeCacheKey received an empty path.");
    }

    const std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(resolvedAbsolutePath);
    return ToLowerInvariant(canonicalPath.native());
}
