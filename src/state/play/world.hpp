#pragma once

#include "../../render/canvas.hpp"
#include "../../entity/character.hpp"

#include "character_manager.hpp"
#include "item_manager.hpp"

namespace game::state::play
{
  class World
  {
  public:
    static constexpr auto ZOOM_MIN = 50.0f;
    static constexpr auto ZOOM_BASE = 100.0f;
    static constexpr auto ZOOM_STEP = 25.0f;
    static constexpr auto ZOOM_MAX = 400.0f;
    static constexpr auto SIZE = glm::vec2{1920, 1080};
    static constexpr auto BOUNDS =
        glm::vec4(SIZE.x * 0.05, SIZE.y * 0.05, SIZE.x - (SIZE.x * 0.05f), SIZE.y - (SIZE.y * 0.05f));

    enum Focus
    {
      CENTER,
      MENU,
      MENU_TOOLS,
      TOOLS
    };

    void update(entity::Character& character, entity::Cursor& cursor, Canvas& canvas, Focus = CENTER);
    void character_focus(entity::Character& character, Canvas& canvas, Focus = CENTER);
    void set(entity::Character& character, Canvas& canvas, Focus = CENTER);
    glm::vec2 screen_to_world(glm::vec2 screenPosition, const Canvas& canvas) const;
  };
}
