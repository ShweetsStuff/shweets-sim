#pragma once

#include "../../resources.hpp"
#include "../settings_menu.hpp"

namespace game::state::select
{
  class Characters
  {
  public:
    SettingsMenu settingsMenu;

    void update(Resources&, int& characterIndex);
  };
}
