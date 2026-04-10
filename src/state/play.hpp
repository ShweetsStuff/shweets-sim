#pragma once

#include "../resources.hpp"

#include "play/area_manager.hpp"
#include "play/character_manager.hpp"
#include "play/info.hpp"
#include "play/item_manager.hpp"
#include "play/menu.hpp"
#include "play/text.hpp"
#include "play/toasts.hpp"
#include "play/tools.hpp"
#include "play/world.hpp"

namespace game::state
{
  class Play
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

    play::Info info;
    play::Menu menu;
    play::Tools tools;
    play::Text text;
    play::World world;
    play::Toasts toasts;
    play::ItemManager itemManager{};
    play::CharacterManager characterManager{};
    play::AreaManager areaManager{};

    int characterIndex{};
    int areaIndex{};

    float autosaveTime{};
    int cheatCodeIndex{};
    double cheatCodeStartTime{};

    bool isWindows{true};

    bool isStartBegin{};
    bool isStart{};
    bool isStartEnd{};

    bool isEndBegin{};
    bool isEnd{};
    bool isEndEnd{};

    bool isPostgame{};

    Canvas worldCanvas{play::World::SIZE};

    Play() = default;
    void set(Resources&, int characterIndex, Game = CONTINUE);
    void exit(Resources& resources);
    void update(Resources&);
    void tick(Resources&);
    void render(Resources&, Canvas&);
    void save(Resources&);
    play::World::Focus focus_get();
    void start_sequence_begin();
    void end_sequence_begin();
  };
};
