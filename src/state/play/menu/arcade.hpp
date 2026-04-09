#pragma once

#include "arcade/dungeon.hpp"
#include "arcade/orbit.hpp"
#include "arcade/skill_check.hpp"
#include "toasts.hpp"

namespace game::state::play::menu
{
  class Arcade
  {
  public:
    enum Game
    {
      SKILL_CHECK,
      DUNGEON,
      ORBIT
    };

    enum State
    {
      MENU,
      GAMEPLAY,
      INFO
    };

    arcade::SkillCheck skillCheck{};
    arcade::Dungeon dungeon{};
    arcade::Orbit orbit{};
    Game game{SKILL_CHECK};
    State state{MENU};

    Arcade() = default;
    Arcade(entity::Character&);

    void game_reset(entity::Character&, Game);
    void tick();
    void update(Resources&, entity::Character&, entity::Cursor&, Inventory&, Text&, Toasts&);
  };
}
