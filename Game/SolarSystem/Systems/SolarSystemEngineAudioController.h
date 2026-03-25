//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_SOLARSYSTEMENGINEAUDIOCONTROLLER_H
#define PINGPONG_SOLARSYSTEMENGINEAUDIOCONTROLLER_H

#include <cstdint>

class AudioSystem;

enum class SolarEngineAudioState : std::uint8_t {
    Idle = 0,
    Starting,
    Working
};

class SolarSystemEngineAudioController final {
public:
    void Initialize(AudioSystem &audio);

    void Update(AudioSystem &audio, bool movementInputActive, float deltaTime);

    void Shutdown(AudioSystem &audio) noexcept;

private:
    SolarEngineAudioState m_state{SolarEngineAudioState::Idle};
    float m_startElapsed{0.0f};
};


#endif //PINGPONG_SOLARSYSTEMENGINEAUDIOCONTROLLER_H
