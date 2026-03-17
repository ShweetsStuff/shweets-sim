#pragma once

#include "../render/canvas.hpp"

#include "select/characters.hpp"
#include "select/info.hpp"
#include "select/preview.hpp"

namespace game::state
{
  class Select
  {
  public:
    select::Characters characters{};
    select::Info info{};
    select::Preview preview{};

    int characterIndex{-1};

    void tick();
    void update(Resources&);
    void render(Resources&, Canvas&);
  };
};
