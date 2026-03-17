#pragma once

#include <imgui.h>

#include "../../util/imgui/window_slide.hpp"
#include "world.hpp"

namespace game::state::play
{
  class Tools
  {
  public:
    bool isOpen{};
    util::imgui::WindowSlide slide{0.125f, 0.0f};

    void update(entity::Character&, entity::Cursor&, World&, World::Focus, Canvas&);
  };
}
