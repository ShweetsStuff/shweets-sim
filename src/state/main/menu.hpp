#pragma once

#include <imgui.h>

#include "../configuration.hpp"

#include "chat.hpp"
#include "cheats.hpp"
#include "debug.hpp"
#include "play.hpp"
#include "stats.hpp"
#include "text.hpp"

#include "../../util/imgui/window_slide.hpp"

namespace game::state::main
{
  class Menu
  {
  public:
    Play play;
    Chat chat;
    Cheats cheats;
    Debug debug;
    Stats stats;
    Inventory inventory;

    state::Configuration configuration;

#if DEBUG
    bool isCheats{};
    bool isDebug{true};
#else
    bool isCheats{};
    bool isDebug{};
#endif

    bool isOpen{true};
    bool isChat{true};
    util::imgui::WindowSlide slide{};

    void tick();
    void update(Resources&, ItemManager&, entity::Character&, entity::Cursor&, Text&, Canvas&);
  };
}
