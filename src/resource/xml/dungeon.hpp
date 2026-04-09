#pragma once

#include "../../util/physfs.hpp"

#include <string>

namespace game::resource::xml
{
  class Dungeon
  {
  public:
    std::string title{"Dungeon"};
    std::string description{"Template dungeon schema."};
    bool isValid{};

    Dungeon() = default;
    Dungeon(const util::physfs::Path&);

    bool is_valid() const;
  };
}
