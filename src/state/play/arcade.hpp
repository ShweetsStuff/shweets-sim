#pragma once

#include "arcade/skill_check.hpp"

namespace game::state::play
{
  class Arcade
  {
  public:
    enum View
    {
      MENU,
      SKILL_CHECK,
      SKILL_CHECK_STATS
    };

    SkillCheck skillCheck{};
    View view{MENU};

    Arcade() = default;
    Arcade(entity::Character&);

    void tick();
    void update(Resources&, entity::Character&, Inventory&, Text&);
  };
}
