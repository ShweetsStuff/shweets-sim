// Handles animation entries in .xml files. "Weight" value determines weight of being randomly selected.

#pragma once

#include <string>
#include <vector>

namespace game::resource::xml
{
  class AnimationEntry
  {
  public:
    std::string animation{};
    float weight{1.0f};
  };

  class AnimationEntryCollection : public std::vector<AnimationEntry>
  {
  public:
    std::string* get();
  };

}
