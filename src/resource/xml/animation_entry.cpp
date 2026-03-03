#include "animation_entry.hpp"

#include "../../util/vector.hpp"

namespace game::resource::xml
{
  std::string* AnimationEntryCollection::get()
  {
    if (empty()) return nullptr;
    return &at(util::vector::random_index_weighted(*this, [](const auto& entry) { return entry.weight; })).animation;
  }
}
