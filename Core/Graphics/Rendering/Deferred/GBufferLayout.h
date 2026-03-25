#ifndef PINGPONG_GBUFFERLAYOUT_H
#define PINGPONG_GBUFFERLAYOUT_H

#include <d3d11.h>

#include <cstdint>

constexpr std::uint32_t kGBufferColorSurfaceCount = 3u;

constexpr DXGI_FORMAT kGBufferAlbedoOcclusionFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

constexpr DXGI_FORMAT kGBufferNormalFormat = DXGI_FORMAT_R10G10B10A2_UNORM;

constexpr DXGI_FORMAT kGBufferMaterialFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

enum class GBufferColorSlot : std::uint32_t
{
    AlbedoOcclusion = 0u,
    Normal = 1u,
    Material = 2u
};

#endif
