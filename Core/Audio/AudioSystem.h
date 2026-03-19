//
// Created by SyperOlao on 19.03.2026.
//

#ifndef PINGPONG_AUDIOSYSTEM_H
#define PINGPONG_AUDIOSYSTEM_H

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include <directxtk/Audio.h>

class AudioSystem final
{
public:
    AudioSystem();
    ~AudioSystem();

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    AudioSystem(AudioSystem&&) = delete;
    AudioSystem& operator=(AudioSystem&&) = delete;

    void Initialize();
    void Shutdown();
    void Update();

    void Load(std::string id, const std::filesystem::path& filePath);

    void PlayOneShot(
        std::string_view id,
        float volume = 1.0f,
        float pitch = 0.0f,
        float pan = 0.0f
    );

    void StartLoop(
        std::string_view id,
        float volume = 1.0f,
        float pitch = 0.0f,
        float pan = 0.0f
    );

    void StopLoop(std::string_view id);
    void StopAllLoops();

    [[nodiscard]] bool IsLoopPlaying(std::string_view id) const;

private:
    [[nodiscard]] DirectX::SoundEffect& GetEffect(std::string_view id);
    [[nodiscard]] const DirectX::SoundEffectInstance* GetLoopInstance(std::string_view id) const;
    static std::filesystem::path ResolveAssetPath(const std::filesystem::path& filePath);

private:
    std::unique_ptr<DirectX::AudioEngine> m_engine;
    std::unordered_map<std::string, std::unique_ptr<DirectX::SoundEffect>> m_effects;
    std::unordered_map<std::string, std::unique_ptr<DirectX::SoundEffectInstance>> m_loopInstances;
    bool m_initialized{false};
};


#endif //PINGPONG_AUDIOSYSTEM_H