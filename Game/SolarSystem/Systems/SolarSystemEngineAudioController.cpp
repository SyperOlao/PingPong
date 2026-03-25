//
// Created by SyperOlao on 25.03.2026.
//

#include "SolarSystemEngineAudioController.h"

#include <stdexcept>
#include <string>
#include <string_view>

#include "Core/Audio/AudioSystem.h"

namespace {
    constexpr std::string_view kEngineStartSoundId = "solar_engine_start";
    constexpr std::string_view kEngineWorkSoundId = "solar_engine_work";
    constexpr std::string_view kEngineStopSoundId = "solar_engine_stop";
    constexpr std::string_view kAmbientMusicId = "solar_ambient_music";

    constexpr const char *kEngineStartSoundPath = "Game/SolarSystem/Assets/EngineStart.wav";
    constexpr const char *kEngineWorkSoundPath = "Game/SolarSystem/Assets/EngineWork.wav";
    constexpr const char *kEngineStopSoundPath = "Game/SolarSystem/Assets/EngineStop.wav";
    constexpr const char *kAmbientMusicPath = "Game/SolarSystem/Assets/AmbientSpaceFinal.wav";

    constexpr float kEngineStartDurationSeconds = 0.50f;

    constexpr float kEngineStartVolume = 0.90f;
    constexpr float kEngineWorkVolume = 0.55f;
    constexpr float kEngineStopVolume = 0.90f;
    constexpr float kAmbientMusicVolume = 0.35f;
}

void SolarSystemEngineAudioController::Initialize(AudioSystem &audio) {
    audio.Load(std::string{kEngineStartSoundId}, kEngineStartSoundPath);
    audio.Load(std::string{kEngineWorkSoundId}, kEngineWorkSoundPath);
    audio.Load(std::string{kEngineStopSoundId}, kEngineStopSoundPath);
    audio.Load(std::string{kAmbientMusicId}, kAmbientMusicPath);

    if (!audio.IsLoopPlaying(kAmbientMusicId)) {
        audio.StartLoop(kAmbientMusicId, kAmbientMusicVolume);
    }

    m_state = SolarEngineAudioState::Idle;
    m_startElapsed = 0.0f;
}

void SolarSystemEngineAudioController::Update(
    AudioSystem &audio,
    const bool movementInputActive,
    const float deltaTime) {
    switch (m_state) {
        case SolarEngineAudioState::Idle: {
            if (movementInputActive) {
                audio.PlayOneShot(kEngineStartSoundId, kEngineStartVolume);
                m_startElapsed = 0.0f;
                m_state = SolarEngineAudioState::Starting;
            }
            break;
        }

        case SolarEngineAudioState::Starting: {
            if (!movementInputActive) {
                audio.PlayOneShot(kEngineStopSoundId, kEngineStopVolume);
                m_startElapsed = 0.0f;
                m_state = SolarEngineAudioState::Idle;
                break;
            }

            m_startElapsed += deltaTime;

            if (m_startElapsed >= kEngineStartDurationSeconds) {
                if (!audio.IsLoopPlaying(kEngineWorkSoundId)) {
                    audio.StartLoop(kEngineWorkSoundId, kEngineWorkVolume);
                }

                m_state = SolarEngineAudioState::Working;
            }

            break;
        }

        case SolarEngineAudioState::Working: {
            if (!movementInputActive) {
                audio.StopLoop(kEngineWorkSoundId);
                audio.PlayOneShot(kEngineStopSoundId, kEngineStopVolume);

                m_startElapsed = 0.0f;
                m_state = SolarEngineAudioState::Idle;
            }

            break;
        }

        default:
            break;
    }
}

void SolarSystemEngineAudioController::Shutdown(AudioSystem &audio) noexcept {
    audio.StopLoop(kEngineWorkSoundId);
    audio.StopLoop(kAmbientMusicId);

    m_state = SolarEngineAudioState::Idle;
    m_startElapsed = 0.0f;
}
