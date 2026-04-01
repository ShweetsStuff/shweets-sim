#include "interact.hpp"

#include "../../util/imgui/widget.hpp"
#include "../../util/measurement.hpp"

using namespace game::resource;
using namespace game::resource::xml;
using namespace game::util;
using namespace game::util::imgui;

namespace game::state::play
{
  void Interact::update(Resources& resources, Text& text, entity::Character& character)
  {
    auto& dialogue = character.data.dialogue;
    auto& strings = character.data.strings;
    auto size = ImGui::GetContentRegionAvail();

    ImGui::PushFont(ImGui::GetFont(), resource::Font::HEADER_2);

    if (dialogue.random.is_valid())
      if (WIDGET_FX(ImGui::Button(strings.get(Strings::InteractChatButton).c_str(), ImVec2(size.x, 0))))
        text.set(dialogue.get(dialogue.random), character);

    ImGui::PopFont();

    if (dialogue.help.is_valid())
      if (WIDGET_FX(ImGui::Button(strings.get(Strings::InteractHelpButton).c_str(), ImVec2(size.x, 0))))
        text.set(dialogue.get(dialogue.help), character);

    auto stage = glm::clamp(0, character.stage_get(), character.stage_max_get());
    auto& pool = stage > 0 ? character.data.stages.at(stage - 1).pool : character.data.pool;

    if (pool.is_valid())
      if (WIDGET_FX(
              ImGui::Button(strings.get(Strings::InteractFeelingButton).c_str(), ImVec2(size.x, 0))))
        text.set(dialogue.get(pool), character);

    ImGui::PushFont(ImGui::GetFont(), resource::Font::HEADER_1);
    ImGui::SeparatorText(character.data.name.c_str());
    ImGui::PopFont();

    auto& system = resources.settings.measurementSystem;
    auto weight = character.weight_get(system);
    auto weightUnit = system == measurement::IMPERIAL ? "lbs" : "kg";

    ImGui::Text(strings.get(Strings::InteractWeightFormat).c_str(), weight, weightUnit,
                character.stage_get() + 1);
    ImGui::Text(strings.get(Strings::InteractCapacityFormat).c_str(), character.capacity,
                character.max_capacity());
    ImGui::Text(strings.get(Strings::InteractDigestionRateFormat).c_str(), character.digestion_rate_get());
    ImGui::Text(strings.get(Strings::InteractEatingSpeedFormat).c_str(), character.eatSpeed);
    ImGui::Separator();
    ImGui::Text(strings.get(Strings::InteractTotalCaloriesFormat).c_str(), character.totalCaloriesConsumed);
    ImGui::Text(strings.get(Strings::InteractTotalFoodItemsFormat).c_str(), character.totalFoodItemsEaten);
  }
}
