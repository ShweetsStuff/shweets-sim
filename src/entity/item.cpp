#include "item.hpp"

#include <imgui.h>

#include "../util/vector.hpp"

using game::resource::xml::Anm2;
using namespace game::util;
using namespace glm;

namespace game::entity
{
  Item::Item(Anm2 _anm2, glm::ivec2 _position, int _schemaID, int _durability, int _animationIndex, glm::vec2 _velocity,
             float _rotation)
      : Actor(_anm2, _position, SET, 0.0f, _animationIndex), schemaID(_schemaID), durability(_durability),
        velocity(_velocity)
  {

    rotationOverrideID =
        vector::push_index(overrides, Override(-1, Anm2::ROOT, Override::SET, Anm2::FrameOptional{.rotation = _rotation}));
  }

  void Item::update()
  {
    auto& rotationOverride = overrides[rotationOverrideID];

    position += velocity;

    *rotationOverride.frame.rotation += angularVelocity;
  }
}
