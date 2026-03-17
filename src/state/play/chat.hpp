#pragma once

#include "text.hpp"

#include <imgui.h>

namespace game::state::play
{
  class Chat
  {
  public:
    void update(Resources&, Text&, entity::Character&);
  };
}
