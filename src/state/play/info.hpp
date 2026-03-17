#pragma once

#include "../../entity/character.hpp"
#include "../../resources.hpp"

#include <imgui.h>

namespace game::state::play
{
  class Info
  {
  public:
    void update(Resources&, entity::Character&);
  };
}
