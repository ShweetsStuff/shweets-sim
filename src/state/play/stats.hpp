#pragma once

#include "../../entity/character.hpp"
#include "../../resources.hpp"

#include "arcade/skill_check.hpp"

#include <imgui.h>

namespace game::state::play
{
  class Stats
  {
  public:
    void update(Resources&, SkillCheck&, entity::Character&);
  };
}
