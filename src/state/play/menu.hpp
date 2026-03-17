#pragma once

#include <imgui.h>

#include "../settings_menu.hpp"

#include "arcade/skill_check.hpp"
#include "chat.hpp"
#include "cheats.hpp"
#include "debug.hpp"
#include "stats.hpp"
#include "text.hpp"

#include "../../util/imgui/window_slide.hpp"

namespace game::state::play
{
  class Menu
  {
  public:
    SkillCheck skillCheck;
    Chat chat;
    Cheats cheats;
    Debug debug;
    Stats stats;
    Inventory inventory;

    state::SettingsMenu settingsMenu;

#if DEBUG
    bool isCheats{true};
#elif
    bool isCheats{};
#endif

    bool isOpen{true};
    bool isChat{true};
    util::imgui::WindowSlide slide{};

    void tick();
    void update(Resources&, ItemManager&, entity::Character&, entity::Cursor&, Text&, Canvas&);
    void color_set_check(Resources&, entity::Character&);
  };
}
