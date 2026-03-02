#pragma once

#include "../../util/physfs.hpp"
#include "../font.hpp"
#include "sound_entry.hpp"

namespace game::resource::xml
{
  class Menu
  {
  public:
    struct Sounds
    {
      SoundEntryCollection open{};
      SoundEntryCollection close{};
      SoundEntryCollection hover{};
      SoundEntryCollection select{};
      SoundEntryCollection cheatsActivated{};
    };

    Sounds sounds{};
    Font font{};
    float rounding{};

    bool isValid{};

    Menu() = default;
    Menu(const util::physfs::Path&);

    bool is_valid() const;
  };
}
