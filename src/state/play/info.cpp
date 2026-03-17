#include "info.hpp"

#include "../../util/color.hpp"
#include "../../util/imgui.hpp"
#include "../../util/math.hpp"
#include "../../util/string.hpp"

#include <algorithm>
#include <format>

using namespace game::resource;
using namespace game::util;

namespace game::state::play
{
  void Info::update(Resources& resources, entity::Character& character)
  {
    static constexpr auto WIDTH_MULTIPLIER = 0.30f;
    static constexpr auto HEIGHT_MULTIPLIER = 4.0f;

    auto& style = ImGui::GetStyle();
    auto windowSize = imgui::to_ivec2(ImGui::GetMainViewport()->Size);

    auto size = ImVec2(windowSize.x * WIDTH_MULTIPLIER - (style.WindowPadding.x * 2.0f),
                       ImGui::GetTextLineHeightWithSpacing() * HEIGHT_MULTIPLIER);
    auto pos = ImVec2((windowSize.x * 0.5f) - (size.x * 0.5f), style.WindowPadding.y);

    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowPos(pos);

    if (ImGui::Begin("##Info", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove))
    {
      auto childSize = ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y);

      if (ImGui::BeginChild("##Weight", childSize))
      {
        auto& system = resources.settings.measurementSystem;
        auto weight = character.weight_get(system);
        auto stage = character.stage_get();
        auto stageMax = character.stage_max_get();
        auto stageWeight = character.stage_threshold_get(stage, system);
        auto stageNextWeight = character.stage_threshold_next_get(system);
        auto unitString = (system == measurement::IMPERIAL ? "lbs" : "kg");

        auto weightString = util::string::format_commas(weight, 2) + " " + unitString;
        ImGui::PushFont(ImGui::GetFont(), Font::ABOVE_AVERAGE);
        ImGui::TextUnformatted(weightString.c_str());
        ImGui::SetItemTooltip("%s", weightString.c_str());
        ImGui::PopFont();

        auto stageProgress = character.stage_progress_get();
        ImGui::ProgressBar(stageProgress, ImVec2(ImGui::GetContentRegionAvail().x, 0),
                           stage >= stageMax ? "MAX" : "To Next Stage");
        if (ImGui::BeginItemTooltip())
        {
          ImGui::Text("Stage: %i/%i (%0.1f%%)", stage + 1, stageMax + 1, math::to_percent(stageProgress));
          ImGui::Separator();
          ImGui::PushStyleColor(ImGuiCol_Text, imgui::to_imvec4(color::GRAY));
          if (stage >= stageMax)
            ImGui::Text("Maxed out!");
          else
          {
            ImGui::Text("Start: %0.2f %s", stageWeight, unitString);
            ImGui::Text("Current: %0.2f %s", weight, unitString);
            ImGui::Text("Next: %0.2f %s", stageNextWeight, unitString);
          }
          ImGui::PopStyleColor();
          ImGui::EndTooltip();
        }
      }
      ImGui::EndChild();

      ImGui::SameLine();

      if (ImGui::BeginChild("##Calories and Capacity", childSize))
      {
        auto& calories = character.calories;
        auto& capacity = character.capacity;

        auto overstuffedPercent = std::max(0.0f, (calories - capacity) / (character.max_capacity() - capacity));
        auto caloriesColor = ImVec4(1.0f, 1.0f - overstuffedPercent, 1.0f - overstuffedPercent, 1.0f);

        ImGui::PushFont(ImGui::GetFont(), Font::ABOVE_AVERAGE);
        ImGui::PushStyleColor(ImGuiCol_Text, caloriesColor);
        auto caloriesString = std::format("{:.0f} kcal / {:.0f} kcal", calories,
                                          character.is_over_capacity() ? character.max_capacity() : character.capacity);
        ImGui::TextUnformatted(caloriesString.c_str());
        ImGui::SetItemTooltip("%s", caloriesString.c_str());
        ImGui::PopStyleColor();
        ImGui::PopFont();

        auto digestionProgress = character.isDigesting
                                     ? (float)character.digestionTimer / character.data.digestionTimerMax
                                     : character.digestionProgress / entity::Character::DIGESTION_MAX;
        ImGui::ProgressBar(digestionProgress, ImVec2(ImGui::GetContentRegionAvail().x, 0),
                           character.isDigesting ? "Digesting..." : "Digestion");

        if (ImGui::BeginItemTooltip())
        {
          if (character.isDigesting)
            ImGui::TextUnformatted("Digestion in progress...");
          else if (digestionProgress <= 0.0f)
            ImGui::TextUnformatted("Give food to start digesting!");
          else
            ImGui::Text("%0.2f%%", math::to_percent(digestionProgress));

          ImGui::Separator();

          ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(imgui::to_imvec4(color::GRAY)));

          ImGui::Text("Rate: %0.2f%% / sec", character.digestion_rate_get());
          ImGui::Text("Eating Speed: %0.2fx", character.eatSpeed);

          ImGui::PopStyleColor();

          ImGui::EndTooltip();
        }
      }
      ImGui::EndChild();
    }
    ImGui::End();
  }
}
