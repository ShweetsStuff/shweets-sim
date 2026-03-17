#include "area_manager.hpp"

#include <imgui.h>

using namespace game::resource;
using namespace game::util;

namespace game::state::play
{
  int AreaManager::get(entity::Character& character)
  {
    auto& data = character.data;
    auto& schema = data.areaSchema;
    if (schema.areas.empty()) return -1;

    auto size = (int)data.stages.size();

    for (int i = 0; i < size; i++)
    {
      auto& stage = data.stages[size - i - 1];
      if (stage.areaID != -1) return stage.areaID;
    }

    return -1;
  }
}
