#pragma once

#include <glm/glm.hpp>
#include <imgui.h>

namespace game::util::imgui::style
{
  void widget_set(float rounding = 10.0f);
  void color_set(glm::vec3 color);
}
