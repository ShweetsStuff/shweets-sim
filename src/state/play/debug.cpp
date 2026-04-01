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
    auto& strings = character.data.strings;
    auto cursorPosition = canvas.screen_position_convert(cursor.position);

    ImGui::Text(strings.get(Strings::DebugCursorScreenFormat).c_str(), cursor.position.x, cursor.position.y);
    ImGui::Text(strings.get(Strings::DebugCursorWorldFormat).c_str(), cursorPosition.x, cursorPosition.y);

    ImGui::SeparatorText(strings.get(Strings::DebugAnimations).c_str());
    ImGui::Text(strings.get(Strings::DebugNowPlayingFormat).c_str(), character.animationMapReverse.at(character.animationIndex).c_str());

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

    ImGui::SeparatorText(strings.get(Strings::DebugDialogue).c_str());

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

    WIDGET_FX(ImGui::Checkbox(strings.get(Strings::DebugShowNulls).c_str(), &character.isShowNulls));
    WIDGET_FX(ImGui::Checkbox(strings.get(Strings::DebugShowWorldBounds).c_str(), &isBoundsDisplay));

    if (!itemManager.items.empty())
    {
      ImGui::SeparatorText(strings.get(Strings::DebugItem).c_str());

      for (int i = 0; i < (int)itemManager.items.size(); i++)
      {
        auto& item = itemManager.items[i];
        if (itemManager.heldItemIndex == i) ImGui::TextUnformatted(strings.get(Strings::DebugHeld).c_str());
        ImGui::Text(strings.get(Strings::DebugItemTypeFormat).c_str(), item.schemaID);
        ImGui::Text(strings.get(Strings::DebugItemPositionFormat).c_str(), item.position.x, item.position.y);
        ImGui::Text(strings.get(Strings::DebugItemVelocityFormat).c_str(), item.velocity.x, item.velocity.y);
        ImGui::Text(strings.get(Strings::DebugItemDurabilityFormat).c_str(), item.durability);
        ImGui::Separator();
      }
    }
  }
}
