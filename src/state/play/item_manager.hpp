#pragma once

#include "../../entity/character.hpp"
#include "../../entity/cursor.hpp"
#include "../../entity/item.hpp"

#include "area_manager.hpp"
#include "text.hpp"

namespace game::state::play
{
  class ItemManager
  {
  public:
    static constexpr auto LIMIT = 100;

    std::vector<entity::Item> items{};
    int heldItemIndex{-1};
    int queuedRemoveItemIndex{-1};

    bool isItemHovered{};
    bool isItemHoveredPrevious{};
    bool isJustItemHoveredStopped{};

    bool isItemHeld{};
    bool isItemHeldPrevious{};
    bool isJustItemHeldStopped{};
    bool isJustItemHeld{};
    bool isJustItemThrown{};

    bool isQueueFinishFood{};

    bool isItemFinished{};

    glm::vec2 cursorPositionPrevious{};
    glm::vec2 cursorDeltaPrevious{};

    std::vector<int> queuedItemIDs{};
    std::vector<int> returnItemIDs{};

    void update(entity::Character&, entity::Cursor&, AreaManager&, Text&, const glm::vec4& bounds, Canvas&);
  };
}
