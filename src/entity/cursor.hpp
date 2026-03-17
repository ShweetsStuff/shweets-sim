#pragma once

#include "actor.hpp"

namespace game::entity
{
  class Cursor : public Actor
  {
  public:
    enum State
    {
      DEFAULT,
      HOVER,
      ACTION
    };

    State state{DEFAULT};
    int interactTypeID{-1};

    Cursor() = default;
    Cursor(resource::xml::Anm2&);
    void tick();
    void update();
  };
}
