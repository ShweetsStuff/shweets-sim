#pragma once

#include <imgui.h>

#include "../../entity/character.hpp"

#include "../../resources.hpp"

namespace game::state::play
{
  class Text
  {
    int index{};
    bool isFinished{};

  public:
    static constexpr auto LIFETIME = 10.0f;

    resource::xml::Dialogue::Entry* entry{};

    bool isEnabled{true};
    float time{};

    void set(resource::xml::Dialogue::Entry*, entity::Character&, bool isInterruptible = true);
    void tick(entity::Character&);
    void update(entity::Character&);
    bool is_interruptible() const;
  };
}