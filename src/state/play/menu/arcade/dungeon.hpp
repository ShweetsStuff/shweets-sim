#pragma once

#include "../../../../entity/character.hpp"

#include <vector>

namespace game::state::play::menu::arcade
{
  class Dungeon
  {
  public:
    struct Tile
    {
      enum State
      {
        HIDDEN,
        SHOWN,
        CORPSE
      };

      enum FlagValue
      {
        FLAG_NONE,
        FLAG_1,
        FLAG_2,
        FLAG_3,
        FLAG_4,
        FLAG_5,
        FLAG_6,
        FLAG_7,
        FLAG_8,
        FLAG_9,
        FLAG_10,
        FLAG_11,
        FLAG_12,
        FLAG_13,
        FLAG_MINE,
        FLAG_QUESTION
      };

      enum Value
      {
        VALUE_0,
        VALUE_1,
        VALUE_2,
        VALUE_3,
        VALUE_4,
        VALUE_5,
        VALUE_6,
        VALUE_7,
        VALUE_8,
        VALUE_9,
        VALUE_10,
        VALUE_11,
        VALUE_12,
        VALUE_13,
        MINE = 100,
        SCROLL = 101
      };

      Value value{VALUE_0};
      State state{HIDDEN};
      FlagValue flagValue{FLAG_NONE};
    };

    static constexpr int GRID_ROWS = 13;
    static constexpr int GRID_COLUMNS = 13;

    std::vector<Tile> tiles{};
    int score{};

    int tile_value_get(const Tile&) const;
    int surrounding_value_sum_get(int row, int column) const;
    bool tile_value_counts_toward_sum(const Tile&) const;
    bool tile_is_scroll(const Tile&) const;
    const char* tile_flag_text_get(const Tile&) const;
    void reveal_diamond(int row, int column, int radius);
    void reset(entity::Character&);
    void tick();
    bool update(entity::Character&);
  };
}
