#include "cheats.hpp"

#include <algorithm>
#include <ranges>

#include "../../util/imgui/input_int_ex.hpp"
#include "../../util/imgui/widget.hpp"

using namespace game::util::imgui;
using namespace game::util;

namespace game::state::play
{
  void Cheats::update(Resources&, entity::Character& character, Inventory& inventory, Text& text)
  {
    static constexpr auto FEED_INCREMENT = 100.0f;

    if (ImGui::BeginChild("##Cheats"))
    {

      if (WIDGET_FX(ImGui::Button("Feed")))
      {
        character.calories = std::min(character.calories + FEED_INCREMENT, character.max_capacity());
        character.queue_idle_animation();
      }
      ImGui::SameLine();
      if (WIDGET_FX(ImGui::Button("Starve")))
      {
        character.calories = std::max(0.0f, character.calories - FEED_INCREMENT);
        character.queue_idle_animation();
      }
      ImGui::SameLine();
      if (WIDGET_FX(ImGui::Button("Digest"))) character.digestionProgress = entity::Character::DIGESTION_MAX;

      auto stage = character.stage + 1;
      if (WIDGET_FX(ImGui::SliderInt("Stage", &stage, 1, (int)character.data.stages.size() + 1)))
      {
        character.stage = glm::clamp(0, stage - 1, (int)character.data.stages.size());
        character.weight =
            character.stage == 0 ? character.data.weight : character.data.stages.at(character.stage - 1).threshold;
        character.queue_idle_animation();
      }

      WIDGET_FX(ImGui::SliderFloat("Capacity", &character.capacity, character.data.capacityMin,
                                   character.data.capacityMax, "%0.0f kcal"));

      WIDGET_FX(ImGui::SliderFloat("Digestion Rate", &character.digestionRate, character.data.digestionRateMin,
                                   character.data.digestionRateMax, "%0.2f% / tick"));
      WIDGET_FX(ImGui::SliderFloat("Eat Speed", &character.eatSpeed, character.data.eatSpeedMin,
                                   character.data.eatSpeedMax, "%0.2fx"));

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

      ImGui::SeparatorText("Inventory");

      if (ImGui::BeginChild("##Inventory", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Borders))
      {
        auto& schema = character.data.itemSchema;

        ImGui::PushItemWidth(100);
        for (int i = 0; i < (int)schema.items.size(); i++)
        {
          auto& item = schema.items[i];
          ImGui::PushID(i);
          WIDGET_FX(input_int_range(item.name.c_str(), &inventory.values[i], 0, schema.quantityMax, 1, 5));
          ImGui::SetItemTooltip("%s", item.name.c_str());
          ImGui::PopID();
        }
        ImGui::PopItemWidth();
      }
      ImGui::EndChild();
    }
    ImGui::EndChild();
  }
}
