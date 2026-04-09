#pragma once

#include "../../../render/canvas.hpp"
#include "../../../entity/actor.hpp"
#include "../../../resources.hpp"

#include <imgui.h>
#include <unordered_map>
#include <vector>

namespace game::state::play::menu
{
  class ItemEffectManager
  {
  public:
    enum Mode
    {
      FALL_DOWN,
      SHOOT_UP
    };

    struct Entry
    {
      int id{-1};
      Mode mode{FALL_DOWN};
      ImVec2 position{};
      ImVec2 velocity{};
      float rotation{};
      float rotationVelocity{};
    };

    std::vector<Entry> entries{};
    std::unordered_map<int, entity::Actor> actors{};
    std::unordered_map<int, glm::vec4> rects{};
    std::unordered_map<int, Canvas> canvases{};

    void tick();
    void spawn(int itemID, const resource::xml::Item& itemSchema, const ImVec4& bounds, Mode mode = FALL_DOWN);
    void render(Resources& resources, const resource::xml::Item& itemSchema, const ImVec4& bounds, float deltaTime);
  };
}
