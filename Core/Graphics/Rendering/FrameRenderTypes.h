#ifndef PINGPONG_FRAMERENDERTYPES_H
#define PINGPONG_FRAMERENDERTYPES_H

#include <cstdint>

enum class RenderPassKind : std::uint8_t
{
    None = 0,
    PreFrame,
    ShadowCapture,
    ForwardOpaque,
    ForwardTransparent,
    DeferredGeometry,
    DeferredLighting,
    GBufferDebug,
    PostProcess,
    Particles,
    UserInterface,
    DebugOverlay,
    Present
};

#endif
