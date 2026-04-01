#include "arcade.hpp"

#include "../../util/imgui/widget.hpp"

using namespace game::util::imgui;
using namespace game::resource::xml;

namespace game::state::play
{
  Arcade::Arcade(entity::Character& character) : skillCheck(character) {}

  void Arcade::tick() { skillCheck.tick(); }

  void Arcade::update(Resources& resources, entity::Character& character, Inventory& inventory, Text& text)
  {
    auto available = ImGui::GetContentRegionAvail();
    auto& strings = character.data.strings;

    if (view == SKILL_CHECK)
    {
      if (skillCheck.update(resources, character, inventory, text)) view = MENU;
      return;
    }

    auto buttonHeight = ImGui::GetFrameHeightWithSpacing();
    auto childSize = ImVec2(available.x, std::max(0.0f, available.y - buttonHeight));

    if (ImGui::BeginChild("##Arcade Child", childSize))
    {
      if (view == MENU)
      {
        auto buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

        ImGui::PushFont(ImGui::GetFont(), resource::Font::HEADER_2);
        ImGui::TextUnformatted(strings.get(Strings::ArcadeSkillCheckName).c_str());
        ImGui::PopFont();

        ImGui::Separator();
        ImGui::TextWrapped("%s", strings.get(Strings::ArcadeSkillCheckDescription).c_str());
        ImGui::Separator();

        if (WIDGET_FX(ImGui::Button(strings.get(Strings::ArcadePlayButton).c_str(), ImVec2(buttonWidth, 0))))
          view = SKILL_CHECK;
        ImGui::SameLine();
        if (WIDGET_FX(ImGui::Button(strings.get(Strings::ArcadeStatsButton).c_str(), ImVec2(buttonWidth, 0))))
          view = SKILL_CHECK_STATS;
      }
      else if (view == SKILL_CHECK_STATS)
      {
        auto& schema = character.data.skillCheckSchema;

        ImGui::PushFont(ImGui::GetFont(), resource::Font::HEADER_2);
        ImGui::TextUnformatted(strings.get(Strings::ArcadeSkillCheckName).c_str());
        ImGui::PopFont();

        ImGui::Separator();
        ImGui::Text(strings.get(Strings::ArcadeBestFormat).c_str(), skillCheck.highScore, skillCheck.bestCombo);
        ImGui::Text(strings.get(Strings::ArcadeTotalSkillChecksFormat).c_str(), skillCheck.totalPlays);

        for (int i = 0; i < (int)schema.grades.size(); i++)
        {
          auto& grade = schema.grades[i];
          ImGui::Text("%s: %i", grade.namePlural.c_str(), skillCheck.gradeCounts[i]);
        }

        ImGui::Text(strings.get(Strings::ArcadeAccuracyFormat).c_str(), skillCheck.accuracy_score_get(character));
      }
    }
    ImGui::EndChild();

    if (view == SKILL_CHECK_STATS)
    {
      if (WIDGET_FX(ImGui::Button(strings.get(Strings::ArcadeBackButton).c_str()))) view = MENU;
    }
  }
}
