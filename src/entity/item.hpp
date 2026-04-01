#pragma once

#include "../resource/xml/item.hpp"

#include "actor.hpp"

namespace game::entity
{
  class Item : public Actor
  {
  public:
    bool isToBeDeleted{};
    bool isHeld{};

    int schemaID{};
    int rotationOverrideID{};
    int durability{};

    glm::vec2 velocity{};
    float angularVelocity{};

    Item(resource::xml::Anm2, glm::ivec2 position, int id, int durability = 0, int animationIndex = -1,
         glm::vec2 velocity = {}, float rotation = 0.0f);
    void update();
  };
}
