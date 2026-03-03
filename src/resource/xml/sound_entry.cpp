#include "sound_entry.hpp"

#include "../../util/vector.hpp"

namespace game::resource::xml
{
  Audio* SoundEntryCollection::get()
  {
    if (empty()) return nullptr;
    return &at(util::vector::random_index_weighted(*this, [](const auto& entry) { return entry.weight; })).sound;
  }

  void SoundEntryCollection::play()
  {
    if (empty()) return;
    if (auto audio = get()) audio->play();
  }
}
