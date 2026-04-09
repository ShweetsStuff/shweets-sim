#pragma once

#include "../text.hpp"

#include <imgui.h>

namespace game::state::play::menu
{
  class Interact
  {
  public:
    void update(Resources&, Text&, entity::Character&);
  };
}
