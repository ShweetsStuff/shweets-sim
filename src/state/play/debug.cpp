#include "debug.hpp"

#include "../../util/imgui/widget.hpp"

#include <ranges>

using namespace game::util::imgui;
using namespace game::resource::xml;

namespace game::state::play
{
  void Debug::update(entity::Character& character, entity::Cursor& cursor, ItemManager& itemManager, Canvas& canvas,
                     Text& text)
  {
    auto cursorPosition = canvas.screen_position_convert(cursor.position);

    ImGui::Text("Cursor Pos (Screen): %0.0f, %0.0f", cursor.position.x, cursor.position.y);
    ImGui::Text("Cursor Pos (World): %0.0f, %0.0f", cursorPosition.x, cursorPosition.y);

    ImGui::SeparatorText("Animations");
    ImGui::Text("Now Playing: %s", character.animationMapReverse.at(character.animationIndex).c_str());

    auto childSize = ImVec2(0, ImGui::GetContentRegionAvail().y / 3);

    if (ImGui::BeginChild("##Animations", childSize, ImGuiChildFlags_Borders))
    {
      for (int i = 0; i < (int)character.animations.size(); i++)
      {
        auto& animation = character.animations[i];
        ImGui::PushID(i);
        if (WIDGET_FX(ImGui::Selectable(animation.name.c_str())))
          character.play(animation.name.c_str(), entity::Actor::PLAY_FORCE);
        ImGui::SetItemTooltip("%s", animation.name.c_str());
        ImGui::PopID();
      }
    }
    ImGui::EndChild();

    ImGui::SeparatorText("Dialogue");

    if (ImGui::BeginChild("##Dialogue", childSize, ImGuiChildFlags_Borders))
    {
      for (int i = 0; i < (int)character.data.dialogue.entries.size(); i++)
      {
        auto& entry = character.data.dialogue.entries[i];
        ImGui::PushID(i);
        if (WIDGET_FX(ImGui::Selectable(entry.name.c_str()))) text.set(&entry, character);
        ImGui::SetItemTooltip("%s", entry.name.c_str());
        ImGui::PopID();
      }
    }
    ImGui::EndChild();

    WIDGET_FX(ImGui::Checkbox("Show Nulls (Hitboxes)", &character.isShowNulls));
    WIDGET_FX(ImGui::Checkbox("Show World Bounds", &isBoundsDisplay));

    if (!itemManager.items.empty())
    {
      ImGui::SeparatorText("Item");

      for (int i = 0; i < (int)itemManager.items.size(); i++)
      {
        auto& item = itemManager.items[i];
        if (itemManager.heldItemIndex == i) ImGui::TextUnformatted("Held");
        ImGui::Text("Type: %i", item.schemaID);
        ImGui::Text("Position: %0.0f, %0.0f", item.position.x, item.position.y);
        ImGui::Text("Velocity: %0.0f, %0.0f", item.velocity.x, item.velocity.y);
        ImGui::Text("Durability: %i", item.durability);
        ImGui::Separator();
      }
    }
  }
}
