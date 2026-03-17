#include "stats.hpp"

#include <ranges>

#include "../../util/measurement.hpp"

using namespace game::resource;
using namespace game::util;

namespace game::state::play
{
  void Stats::update(Resources& resources, SkillCheck& skillCheck, entity::Character& character)
  {
    ImGui::PushFont(ImGui::GetFont(), Font::BIG);
    ImGui::TextUnformatted(character.data.name.c_str());
    ImGui::PopFont();

    ImGui::Separator();

    auto& skillCheckSchema = character.data.skillCheckSchema;
    auto& system = resources.settings.measurementSystem;
    auto weight = character.weight_get(system);
    auto weightUnit = system == measurement::IMPERIAL ? "lbs" : "kg";

    ImGui::Text("Weight: %0.2f %s (Stage: %i)", weight, weightUnit, character.stage_get() + 1);
    ImGui::Text("Capacity: %0.0f kcal (Max: %0.0f kcal)", character.capacity, character.max_capacity());
    ImGui::Text("Digestion Rate: %0.2f%%/sec", character.digestion_rate_get());
    ImGui::Text("Eating Speed: %0.2fx", character.eatSpeed);

    ImGui::SeparatorText("Totals");

    ImGui::Text("Total Calories Consumed: %0.0f kcal", character.totalCaloriesConsumed);
    ImGui::Text("Total Food Items Eaten: %i", character.totalFoodItemsEaten);

    ImGui::SeparatorText("Skill Check");

    ImGui::Text("Best: %i pts (%ix)", skillCheck.highScore, skillCheck.bestCombo);
    ImGui::Text("Total Skill Checks: %i", skillCheck.totalPlays);

    for (int i = 0; i < (int)skillCheckSchema.grades.size(); i++)
    {
      auto& grade = skillCheckSchema.grades[i];
      ImGui::Text("%s: %i", grade.namePlural.c_str(), skillCheck.gradeCounts[i]);
    }

    ImGui::Text("Accuracy: %0.2f%%", skillCheck.accuracy_score_get(character));
  }
}
