//
// Created by SyperOlao on 19.03.2026.
//
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "AudioSystem.h"


#include <Audio.h>
#include <Windows.h>

#include <filesystem>
#include <stdexcept>
#include <string>

namespace {
    [[nodiscard]] std::filesystem::path GetExecutableDirectory() {
        std::wstring buffer(MAX_PATH, L'\0');

        DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0) {
            throw std::runtime_error("GetModuleFileNameW failed while resolving audio path.");
        }

        while (length == buffer.size()) {
            buffer.resize(buffer.size() * 2);

            length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
            if (length == 0) {
                throw std::runtime_error("GetModuleFileNameW failed while resolving audio path.");
            }
        }

        buffer.resize(length);
        return std::filesystem::path(buffer).parent_path();
    }
}

AudioSystem::AudioSystem() = default;

AudioSystem::~AudioSystem() {
    Shutdown();
}

void AudioSystem::Initialize() {
    if (m_initialized) {
        return;
    }

    DirectX::AUDIO_ENGINE_FLAGS flags = DirectX::AudioEngine_Default;

#ifdef _DEBUG
    flags |= DirectX::AudioEngine_Debug;
#endif

    m_engine = std::make_unique<DirectX::AudioEngine>(flags);

    if (!m_engine) {
        throw std::runtime_error("Failed to create AudioEngine.");
    }

    if (m_engine->IsCriticalError()) {
        throw std::runtime_error("AudioEngine entered critical error state during initialization.");
    }

    m_initialized = true;
}

void AudioSystem::Shutdown() {
    if (!m_initialized) {
        return;
    }

    StopAllLoops();
    m_loopInstances.clear();
    m_effects.clear();
    m_engine.reset();
    m_initialized = false;
}

void AudioSystem::Update() {
    if (!m_initialized || !m_engine) {
        return;
    }

    const bool ok = m_engine->Update();
    if (!ok && m_engine->IsCriticalError()) {
        throw std::runtime_error("AudioEngine critical error during update.");
    }
}

void AudioSystem::Load(std::string id, const std::filesystem::path &filePath) {
    if (!m_initialized || !m_engine) {
        throw std::runtime_error("AudioSystem::Load called before Initialize.");
    }

    if (id.empty()) {
        throw std::invalid_argument("AudioSystem::Load received empty sound id.");
    }

    const std::filesystem::path resolvedPath = ResolveAssetPath(filePath);

    auto effect = std::make_unique<DirectX::SoundEffect>(m_engine.get(), resolvedPath.c_str());
    m_effects[std::move(id)] = std::move(effect);
}

void AudioSystem::PlayOneShot(
    const std::string_view id,
    const float volume,
    const float pitch,
    const float pan
) {
    GetEffect(id).Play(volume, pitch, pan);
}

void AudioSystem::StartLoop(
    const std::string_view id,
    const float volume,
    const float pitch,
    const float pan
) {
    const std::string key{id};
    auto &instance = m_loopInstances[key];

    if (!instance) {
        instance = GetEffect(id).CreateInstance();
    }

    instance->Stop(true);
    instance->SetVolume(volume);
    instance->SetPitch(pitch);
    instance->SetPan(pan);
    instance->Play(true);
}

void AudioSystem::StopLoop(const std::string_view id) {
    const auto it = m_loopInstances.find(std::string{id});
    if (it == m_loopInstances.end() || !it->second) {
        return;
    }

    it->second->Stop(true);
}

void AudioSystem::StopAllLoops() {
    for (auto &[_, instance]: m_loopInstances) {
        if (instance) {
            instance->Stop(true);
        }
    }
}

bool AudioSystem::IsLoopPlaying(const std::string_view id) const {
    const auto it = m_loopInstances.find(std::string{id});
    if (it == m_loopInstances.end() || !it->second) {
        return false;
    }

    return it->second->GetState() == DirectX::SoundState::PLAYING;
}

DirectX::SoundEffect &AudioSystem::GetEffect(const std::string_view id) {
    const auto it = m_effects.find(std::string{id});
    if (it == m_effects.end() || !it->second) {
        throw std::runtime_error("Sound not loaded: " + std::string{id});
    }

    return *it->second;
}

const DirectX::SoundEffectInstance *AudioSystem::GetLoopInstance(std::string_view id) const {
    const auto it = m_loopInstances.find(std::string{id});
    if (it == m_loopInstances.end() || !it->second) {
        return nullptr;
    }

    return it->second.get();
}

std::filesystem::path AudioSystem::ResolveAssetPath(const std::filesystem::path &filePath) {
    if (filePath.empty()) {
        throw std::invalid_argument("AudioSystem::ResolveAssetPath received empty path.");
    }

    if (std::filesystem::exists(filePath)) {
        return std::filesystem::absolute(filePath);
    }

    std::filesystem::path current = GetExecutableDirectory();

    for (int i = 0; i < 8; ++i) {
        const std::filesystem::path candidate = current / filePath;
        if (std::filesystem::exists(candidate)) {
            return std::filesystem::weakly_canonical(candidate);
        }

        if (!current.has_parent_path()) {
            break;
        }

        const auto parent = current.parent_path();
        if (parent == current) {
            break;
        }

        current = parent;
    }

    throw std::runtime_error("Audio file not found: " + filePath.string());
}
