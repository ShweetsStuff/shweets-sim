#pragma once

#include "../../entity/character.hpp"
#include "../../entity/cursor.hpp"
#include "text.hpp"

namespace game::state::play
{
  class CharacterManager
  {
  public:
    bool isDisabled{};
    bool isInteracting{};
    bool isHovering{};
    bool isInteractingPrevious{};
    bool isHoveringPrevious{};
    bool isJustStoppedInteracting{};
    bool isJustStoppedHovering{};
    bool isHoldInteracting{};
    bool isHoldInteractingPrevious{};
    bool isJustStoppedHoldInteracting{};
    int interactAreaID{-1};

    glm::vec2 cursorWorldPositionPrevious{};
    std::string queuedAnimation{};

    void update(entity::Character&, entity::Cursor&, Text&, Canvas&);
  };
}
