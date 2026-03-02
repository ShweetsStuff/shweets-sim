#pragma once

#include "../resources.hpp"

#include "main/area_manager.hpp"
#include "main/character_manager.hpp"
#include "main/info.hpp"
#include "main/item_manager.hpp"
#include "main/menu.hpp"
#include "main/text.hpp"
#include "main/toasts.hpp"
#include "main/tools.hpp"
#include "main/world.hpp"

namespace game::state
{
  class Main
  {
  public:
    static constexpr auto AUTOSAVE_TIME = 30.0f;

    enum Game
    {
      NEW_GAME,
      CONTINUE
    };

    entity::Character character;
    entity::Cursor cursor;

    main::Info info;
    main::Menu menu;
    main::Tools tools;
    main::Text text;
    main::World world;
    main::Toasts toasts;
    main::ItemManager itemManager{};
    main::CharacterManager characterManager{};
    main::AreaManager areaManager{};

    int characterIndex{};
    int areaIndex{};

    float autosaveTime{};
    int konamiCodeIndex{};
    double konamiCodeStartTime{};

    bool isWindows{true};

    bool isStartBegin{};
    bool isStart{};
    bool isStartEnd{};

    bool isEndBegin{};
    bool isEnd{};
    bool isEndEnd{};

    bool isPostgame{};

    Canvas worldCanvas{main::World::SIZE};

    Main() = default;
    void set(Resources&, int characterIndex, Game = CONTINUE);
    void exit(Resources& resources);
    void update(Resources&);
    void tick(Resources&);
    void render(Resources&, Canvas&);
    void save(Resources&);
    main::World::Focus focus_get();
  };
};
