#include "audio.hpp"

#include <SDL3/SDL_properties.h>

#include "../log.hpp"

#include <string>
#include <format>
#include <unordered_map>

using namespace game::util;

namespace game::resource
{
  static std::shared_ptr<MIX_Audio> audio_make(MIX_Audio* audio)
  {
    return std::shared_ptr<MIX_Audio>(audio,
                                      [](MIX_Audio* a)
                                      {
                                        if (a) MIX_DestroyAudio(a);
                                      });
  }

  static std::unordered_map<std::string, std::weak_ptr<MIX_Audio>> audioCache{};

  static std::shared_ptr<MIX_Audio> cache_get(const std::string& key)
  {
    auto it = audioCache.find(key);
    if (it == audioCache.end()) return {};

    auto cached = it->second.lock();
    if (!cached) audioCache.erase(it);
    return cached;
  }

  static void cache_set(const std::string& key, const std::shared_ptr<MIX_Audio>& audio)
  {
    if (!audio) return;
    audioCache[key] = audio;
  }

  MIX_Mixer* Audio::mixer_get()
  {
    static auto mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    return mixer;
  }

  void Audio::volume_set(float volume)
  {
    auto mixer = mixer_get();
    MIX_SetMasterGain(mixer, volume);
  }

  Audio::Audio(const std::filesystem::path& path)
  {
    auto pathString = path.string();
    auto key = std::string("fs:") + pathString;
    internal = cache_get(key);
    if (internal)
    {
      logger.info(std::format("Using cached audio: {}", pathString));
      return;
    }

    internal = audio_make(MIX_LoadAudio(mixer_get(), pathString.c_str(), true));
    cache_set(key, internal);
    if (internal) logger.info(std::format("Initialized audio: {}", pathString));

    if (!internal) logger.info(std::format("Failed to intialize audio: {} ({})", pathString, SDL_GetError()));
  }

  Audio::Audio(const physfs::Path& path)
  {
    if (!path.is_valid())
    {
      logger.error(
          std::format("Failed to initialize audio from PhysicsFS path: {}", path.c_str(), physfs::error_get()));
      return;
    }

    auto key = std::string("physfs:") + path.c_str();
    internal = cache_get(key);
    if (internal)
    {
      logger.info(std::format("Using cached audio: {}", path.c_str()));
      return;
    }

    auto buffer = path.read();

    if (buffer.empty())
    {
      logger.error(
          std::format("Failed to initialize audio from PhysicsFS path: {} ({})", path.c_str(), physfs::error_get()));
      return;
    }

    auto ioStream = SDL_IOFromConstMem(buffer.data(), buffer.size());

    internal = audio_make(MIX_LoadAudio_IO(mixer_get(), ioStream, false, true));
    cache_set(key, internal);
    if (internal)
      logger.info(std::format("Initialized audio: {}", path.c_str()));
    else
      logger.info(std::format("Failed to intialize audio: {} ({})", path.c_str(), SDL_GetError()));
  }

  Audio::Audio(const Audio& other)
  {
    internal = other.internal;
    track = nullptr;
  }

  Audio::Audio(Audio&& other) noexcept
  {
    internal = std::move(other.internal);
    track = other.track;

    other.track = nullptr;
  }

  Audio& Audio::operator=(const Audio& other)
  {
    if (this != &other)
    {
      unload();
      internal = other.internal;
      track = nullptr;
    }
    return *this;
  }

  Audio& Audio::operator=(Audio&& other) noexcept
  {
    if (this != &other)
    {
      unload();
      internal = std::move(other.internal);
      track = other.track;

      other.track = nullptr;
    }
    return *this;
  }

  void Audio::unload()
  {
    if (track)
    {
      MIX_DestroyTrack(track);
      track = nullptr;
    }
    internal.reset();
  }

  void Audio::play(bool isLoop) const
  {
    if (!internal) return;

    auto mixer = mixer_get();

    if (track && MIX_GetTrackMixer(track) != mixer)
    {
      MIX_DestroyTrack(track);
      track = nullptr;
    }

    if (!track)
    {
      track = MIX_CreateTrack(mixer);
      if (!track) return;
    }

    MIX_SetTrackAudio(track, internal.get());

    SDL_PropertiesID options = 0;

    if (isLoop)
    {
      options = SDL_CreateProperties();
      if (options) SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    }

    MIX_PlayTrack(track, options);

    if (options) SDL_DestroyProperties(options);
  }

  void Audio::stop() const
  {
    if (track) MIX_StopTrack(track, 0);
  }

  bool Audio::is_playing() const { return track && MIX_TrackPlaying(track); }

  Audio::~Audio() { unload(); }
  bool Audio::is_valid() const { return (bool)internal; }
}
