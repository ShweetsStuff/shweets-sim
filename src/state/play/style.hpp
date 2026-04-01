#pragma once

#include "../../entity/character.hpp"
#include "../../resources.hpp"
#include "../../util/imgui/style.hpp"

namespace game::state::play::style
{
  inline void color_set(Resources& resources, const entity::Character& character)
  {
    game::util::imgui::style::color_set(resources.settings.isUseCharacterColor ? character.data.color
                                                                                : resources.settings.color);
  }
}
