#pragma once

#include "menu/inventory.hpp"
#include "text.hpp"

#include <imgui.h>

namespace game::state::play
{
  class Cheats
  {
  public:
    void update(Resources&, entity::Character&, menu::Inventory&);
  };
}
