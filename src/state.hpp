#pragma once

#include <SDL3/SDL.h>

#include "render/canvas.hpp"
#include "resources.hpp"

#include "state/play.hpp"
#include "state/select.hpp"

#include "entity/cursor.hpp"

namespace game
{
  class State
  {
  public:
    SDL_Window* window{};
    SDL_GLContext context{};
    Uint64 previousUpdate{};
    Uint64 previousTick{};

    enum Type
    {
      PLAY,
      SELECT
    };

    Type type{SELECT};

    Resources resources;

    state::Play play;
    state::Select select;

    void tick();
    void tick_60();
    void update();
    void render();

    bool isRunning{true};
    bool isCursorHidden{};

    Canvas canvas{};

    State(SDL_Window*, SDL_GLContext, resource::xml::Settings);
    void loop();
  };
};
