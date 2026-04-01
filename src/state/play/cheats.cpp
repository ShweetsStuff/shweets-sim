#include "cheats.hpp"

#include <algorithm>
#include <ranges>

#include "../../util/imgui/input_int_ex.hpp"
#include "../../util/imgui/widget.hpp"

using namespace game::util::imgui;
using namespace game::util;
using namespace game::resource::xml;

namespace game::state::play
{
  void Cheats::update(Resources&, entity::Character& character, Inventory& inventory)
  {
    auto& strings = character.data.strings;

    if (ImGui::BeginChild("##Cheats"))
    {
      auto stage = character.stage + 1;

      auto weight_update = [&]() { character.queue_idle_animation(); };

      WIDGET_FX(ImGui::SliderFloat(strings.get(Strings::CheatsCalories).c_str(), &character.calories, 0,
                                   character.max_capacity(), "%0.0f kcal"));
      WIDGET_FX(ImGui::SliderFloat(strings.get(Strings::CheatsCapacity).c_str(), &character.capacity,
                                   character.data.capacityMin, character.data.capacityMax, "%0.0f kcal"));

      if (WIDGET_FX(ImGui::SliderFloat(strings.get(Strings::CheatsWeight).c_str(), &character.weight,
                                       character.data.weight, character.data.weightMax,
                                       strings.get(Strings::CheatsWeightFormat).c_str())))
        weight_update();

      if (WIDGET_FX(ImGui::SliderInt(strings.get(Strings::CheatsStage).c_str(), &stage, 1,
                                     (int)character.data.stages.size() + 1)))
      {
        character.stage = glm::clamp(0, stage - 1, (int)character.data.stages.size());
        character.weight =
            character.stage == 0 ? character.data.weight : character.data.stages.at(character.stage - 1).threshold;
        weight_update();
      }

      WIDGET_FX(ImGui::SliderFloat(strings.get(Strings::CheatsDigestionRate).c_str(), &character.digestionRate,
                                   character.data.digestionRateMin, character.data.digestionRateMax,
                                   strings.get(Strings::CheatsDigestionRateFormat).c_str()));
      WIDGET_FX(ImGui::SliderFloat(strings.get(Strings::CheatsEatSpeed).c_str(), &character.eatSpeed,
                                   character.data.eatSpeedMin, character.data.eatSpeedMax,
                                   strings.get(Strings::CheatsEatSpeedFormat).c_str()));

      if (WIDGET_FX(ImGui::Button(strings.get(Strings::CheatsDigestButton).c_str())))
        character.digestionProgress = entity::Character::DIGESTION_MAX;

      ImGui::SeparatorText(strings.get(Strings::CheatsInventory).c_str());

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
