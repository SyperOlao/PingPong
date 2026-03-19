//
// Created by SyperOlao on 19.03.2026.
//

#ifndef PINGPONG_PONGTYPES_H
#define PINGPONG_PONGTYPES_H
#include <cstdint>

enum class Difficulty : std::uint8_t {
    Easy = 0,
    Medium,
    Hard
};

enum class MatchRule : std::uint8_t {
    FirstTo10 = 0,
    Endless
};

struct DifficultyTuning final {
    const char *Label{nullptr};
    float BallStartSpeed{0.0f};
    float PaddleHeight{0.0f};
    float AIPaddleSpeed{0.0f};
    float AIMistakeChancePerSecond{0.0f};
    float AIMistakeDurationMin{0.0f};
    float AIMistakeDurationMax{0.0f};
    float AITrackingError{0.0f};
    float AIMistakeSpeedMultiplier{1.0f};
};

struct AIState final {
    float MistakeTimer{0.0f};
    float CurrentTrackingOffset{0.0f};
};
#endif //PINGPONG_PONGTYPES_H
