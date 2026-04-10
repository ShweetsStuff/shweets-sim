#pragma once

#include <imgui.h>

#include "../settings_menu.hpp"

#include "menu/arcade.hpp"
#include "cheats.hpp"
#include "debug.hpp"
#include "menu/interact.hpp"
#include "menu/inventory.hpp"
#include "text.hpp"
#include "menu/toasts.hpp"

#include "../../util/imgui/window_slide.hpp"

namespace game::state::play
{
  class Menu
  {
  public:
    menu::Arcade arcade;
    menu::Interact interact;
    Cheats cheats;
    Debug debug;
    menu::Inventory inventory;
    menu::Toasts toasts;

    state::SettingsMenu settingsMenu;

#if DEBUG
    bool isCheats{true};
#else
    bool isCheats{};
#endif

    bool isOpen{true};
    bool isDebugOpen{};
    util::imgui::WindowSlide slide{};

    void tick();
    void update(Resources&, ItemManager&, entity::Character&, entity::Cursor&, Text&, Canvas&);
  };
}
