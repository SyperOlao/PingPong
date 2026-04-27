#ifndef PINGPONG_KATAMARISCENELIGHTING_H
#define PINGPONG_KATAMARISCENELIGHTING_H

#include "Core/Graphics/Particles/GpuParticleTypes.h"
#include "Core/Graphics/Rendering/Lighting/SceneLighting3D.h"
#include "Game/Katamari/Data/KatamariGameConfig.h"

[[nodiscard]] SceneLightingDescriptor3D CreateKatamariSceneLighting(const KatamariGameConfig &config);

[[nodiscard]] GpuParticleEmitterDesc CreateKatamariParticleEmitterDesc(const KatamariGameConfig &config);

#endif
