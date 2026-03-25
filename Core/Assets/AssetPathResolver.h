#ifndef PINGPONG_ASSETPATHRESOLVER_H
#define PINGPONG_ASSETPATHRESOLVER_H

#include <filesystem>
#include <string>

class AssetPathResolver final
{
public:
    [[nodiscard]] static std::filesystem::path Resolve(const std::filesystem::path &contentRelativeOrAbsolutePath);

    [[nodiscard]] static std::wstring MakeCacheKey(const std::filesystem::path &resolvedAbsolutePath);

private:
    [[nodiscard]] static std::filesystem::path GetExecutableDirectory();

    AssetPathResolver() = default;
};

#endif
