#pragma once

#include "../../entity/character.hpp"
#include "../../entity/cursor.hpp"

#include "item_manager.hpp"
#include "text.hpp"

#include <imgui.h>

namespace game::state::play
{
  class Debug
  {
  public:
    bool isBoundsDisplay{};

    void update(entity::Character&, entity::Cursor&, ItemManager&, Canvas&, Text&);
  };
}
