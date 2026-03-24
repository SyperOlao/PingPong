//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_SOLARSYSTEMTUNING_H
#define PINGPONG_SOLARSYSTEMTUNING_H

struct SolarSystemTuning final {
    float PlanetRotationScale{1.0f};
    float MoonRotationScale{1.0f};

    float PlanetOrbitRadiusScale{1.0f};
    float MoonOrbitRadiusScale{1.0f};

    float OrbitEccentricityScale{1.0f};

    bool ShowOrbits{true};
    bool ShowBackgroundStars{true};
};

#endif //PINGPONG_SOLARSYSTEMTUNING_H
