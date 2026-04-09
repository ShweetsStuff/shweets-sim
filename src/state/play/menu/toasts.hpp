#pragma once

#include <imgui.h>

#include <string>
#include <vector>

namespace game::state::play::menu
{
  class Toasts
  {
  public:
    struct Toast
    {
      std::string message{};
      ImVec2 position{};
      int time{};
      int timeMax{};
    };

    std::vector<Toast> toasts{};

    void spawn(const std::string& message, const ImVec2& position, int time);
    void update(ImDrawList*);
  };
}
