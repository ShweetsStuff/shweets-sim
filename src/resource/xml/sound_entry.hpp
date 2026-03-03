// Handles sound entries in .xml files. "Weight" value determines weight of being randomly selected.

#pragma once

#include "../audio.hpp"

namespace game::resource::xml
{
  class SoundEntry
  {
  public:
    Audio sound{};
    float weight{1.0f};

    inline void play() { sound.play(); };
  };

  class SoundEntryCollection : public std::vector<SoundEntry>
  {
  public:
    Audio* get();
    void play();
  };
}
