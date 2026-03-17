#pragma once

#include "../../entity/character.hpp"

namespace game::state::play
{
  class AreaManager
  {
  public:
    int get(entity::Character&);
  };
}
