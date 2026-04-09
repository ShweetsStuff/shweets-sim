#pragma once

#include <SDL3_mixer/SDL_mixer.h>
#include <filesystem>
#include <memory>

#include "../util/physfs.hpp"

namespace game::resource
{
  class Audio
  {
    static MIX_Mixer* mixer_get();
    void unload();

    std::shared_ptr<MIX_Audio> internal{};
    mutable MIX_Track* track{nullptr};

  public:
    Audio() = default;
    Audio(const std::filesystem::path&);
    Audio(const util::physfs::Path&);
    Audio(const Audio&);
    Audio(Audio&&) noexcept;
    Audio& operator=(const Audio&);
    Audio& operator=(Audio&&) noexcept;
    ~Audio();
    bool is_valid() const;
    void play(bool isLoop = false) const;
    void stop() const;
    bool is_playing() const;
    static void volume_set(float volume);
  };
}
