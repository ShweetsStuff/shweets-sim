#pragma once

#include <imgui.h>

#include "../settings_menu.hpp"

#include "arcade.hpp"
#include "cheats.hpp"
#include "debug.hpp"
#include "interact.hpp"
#include "inventory.hpp"
#include "text.hpp"

#include "../../util/imgui/window_slide.hpp"

namespace game::state::play
{
  class Menu
  {
  public:
    Arcade arcade;
    Interact interact;
    Cheats cheats;
    Debug debug;
    Inventory inventory;

    state::SettingsMenu settingsMenu;

#if DEBUG
    bool isCheats{true};
#else
    bool isCheats{};
#endif

    bool isOpen{true};
    util::imgui::WindowSlide slide{};

    void tick();
    void update(Resources&, ItemManager&, entity::Character&, entity::Cursor&, Text&, Canvas&);
  };
}
