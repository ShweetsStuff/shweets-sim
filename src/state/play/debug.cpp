#include "debug.hpp"

#include "../../util/imgui/widget.hpp"

#include <ranges>

using namespace game::util::imgui;

namespace game::state::play
{
  void Debug::update(entity::Character& character, entity::Cursor& cursor, ItemManager& itemManager, Canvas& canvas)
  {
    auto cursorPosition = canvas.screen_position_convert(cursor.position);

    ImGui::Text("Cursor Pos (Screen): %0.0f, %0.0f", cursor.position.x, cursor.position.y);
    ImGui::Text("Cursor Pos (World): %0.0f, %0.0f", cursorPosition.x, cursorPosition.y);

    WIDGET_FX(ImGui::Checkbox("Show Nulls (Hitboxes)", &character.isShowNulls));
    WIDGET_FX(ImGui::Checkbox("Show World Bounds", &isBoundsDisplay));

    if (!itemManager.items.empty())
    {
      ImGui::SeparatorText("Item");

      for (int i = 0; i < (int)itemManager.items.size(); i++)
      {
        auto& item = itemManager.items[i];
        if (itemManager.heldItemIndex == i) ImGui::Text("Held");
        ImGui::Text("Type: %i", item.schemaID);
        ImGui::Text("Position: %0.0f, %0.0f", item.position.x, item.position.y);
        ImGui::Text("Velocity: %0.0f, %0.0f", item.velocity.x, item.velocity.y);
        ImGui::Text("Chew Count: %i", item.chewCount);
        ImGui::Separator();
      }
    }
  }
}
