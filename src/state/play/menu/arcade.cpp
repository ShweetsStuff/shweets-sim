#include "arcade.hpp"

#include "../../../util/imgui/widget.hpp"

using namespace game::util::imgui;
using namespace game::resource::xml;

namespace game::state::play::menu
{
  namespace
  {
    struct GameInfoStrings
    {
      Strings::Type name;
      Strings::Type description;
      Strings::Type howToPlay;
    };
  }

  Arcade::Arcade(entity::Character& character) : skillCheck(character) {}

  void Arcade::game_reset(entity::Character& character, Game gameCurrent)
  {
    switch (gameCurrent)
    {
      case SKILL_CHECK:
        skillCheck.reset(character);
        break;
      case DUNGEON:
        dungeon.reset(character);
        break;
      case ORBIT:
        orbit.reset(character);
        break;
    }
  }

  void Arcade::tick()
  {
    skillCheck.tick();
    dungeon.tick();
    orbit.tick();
  }

  void Arcade::update(Resources& resources, entity::Character& character, entity::Cursor& cursor, Inventory& inventory,
                      Text& text, Toasts& toasts)
  {
    auto available = ImGui::GetContentRegionAvail();
    auto& strings = character.data.strings;
    auto game_info_strings_get = [&](Game gameCurrent) -> GameInfoStrings
    {
      switch (gameCurrent)
      {
        case SKILL_CHECK:
          return {Strings::ArcadeSkillCheckName, Strings::ArcadeSkillCheckDescription,
                  Strings::ArcadeSkillCheckHowToPlay};
        case DUNGEON:
          return {Strings::ArcadeDungeonName, Strings::ArcadeDungeonDescription, Strings::ArcadeDungeonHowToPlay};
        case ORBIT:
          return {Strings::ArcadeOrbitName, Strings::ArcadeOrbitDescription, Strings::ArcadeOrbitHowToPlay};
      }

      return {Strings::ArcadeSkillCheckName, Strings::ArcadeSkillCheckDescription, Strings::ArcadeSkillCheckHowToPlay};
    };
    auto game_header_draw = [&](Game gameCurrent)
    {
      auto gameInfoStrings = game_info_strings_get(gameCurrent);
      ImGui::PushFont(ImGui::GetFont(), resource::Font::HEADER_2);
      ImGui::TextUnformatted(strings.get(gameInfoStrings.name).c_str());
      ImGui::PopFont();
    };

    auto game_menu_draw = [&](Game gameCurrent)
    {
      constexpr auto GAME_CHILD_HEIGHT_MULTIPLIER = 7.0f;
      constexpr auto GAME_DESCRIPTION_HEIGHT_MULTIPLIER = 4.75f;

      auto lineHeight = ImGui::GetTextLineHeightWithSpacing();
      auto gameChildHeight = lineHeight * GAME_CHILD_HEIGHT_MULTIPLIER;
      auto gameDescriptionHeight = lineHeight * GAME_DESCRIPTION_HEIGHT_MULTIPLIER;
      auto gameInfoStrings = game_info_strings_get(gameCurrent);
      auto detailsChildID = [gameCurrent]()
      {
        switch (gameCurrent)
        {
          case SKILL_CHECK:
            return "##ArcadeSkillCheckDescription";
          case DUNGEON:
            return "##ArcadeDungeonDescription";
          case ORBIT:
            return "##ArcadeOrbitDescription";
        }

        return "##ArcadeDescription";
      }();

      if (ImGui::BeginChild(gameInfoStrings.name, {0, gameChildHeight}, ImGuiChildFlags_Borders))
      {
        auto buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

        ImGui::BeginChild(detailsChildID, {0, gameDescriptionHeight});
        game_header_draw(gameCurrent);
        ImGui::Separator();
        ImGui::TextWrapped("%s", strings.get(gameInfoStrings.description).c_str());
        ImGui::EndChild();

        ImGui::Separator();

        if (WIDGET_FX(ImGui::Button(strings.get(Strings::ArcadePlayButton).c_str(), ImVec2(buttonWidth, 0))))
        {
          game_reset(character, gameCurrent);
          game = gameCurrent;
          state = GAMEPLAY;
        }
        ImGui::SameLine();
        if (WIDGET_FX(ImGui::Button(strings.get(Strings::ArcadeInfoButton).c_str(), ImVec2(buttonWidth, 0))))
        {
          game = gameCurrent;
          state = INFO;
        }
      }
      ImGui::EndChild();
    };

    auto game_info_sections_draw = [&](Game gameCurrent)
    {
      auto gameInfoStrings = game_info_strings_get(gameCurrent);

      ImGui::PushFont(ImGui::GetFont(), resource::Font::HEADER_1);
      ImGui::TextWrapped("%s", strings.get(Strings::ArcadeHowToPlay).c_str());
      ImGui::PopFont();
      ImGui::Separator();
      ImGui::PushFont(ImGui::GetFont(), resource::Font::NORMAL);
      ImGui::TextWrapped("%s", strings.get(gameInfoStrings.howToPlay).c_str());
      ImGui::PopFont();

      ImGui::PushFont(ImGui::GetFont(), resource::Font::HEADER_1);
      ImGui::TextWrapped("%s", strings.get(Strings::ArcadeStats).c_str());
      ImGui::PopFont();
      ImGui::Separator();
    };

    auto game_stats_draw = [&](Game gameCurrent)
    {
      switch (gameCurrent)
      {
        case SKILL_CHECK:
        {
          auto& schema = character.data.skillCheckSchema;

          ImGui::Text(strings.get(Strings::ArcadeBestScoreComboFormat).c_str(), skillCheck.highScore,
                      skillCheck.bestCombo);
          ImGui::Text(strings.get(Strings::ArcadeTotalSkillChecksFormat).c_str(), skillCheck.totalPlays);

          for (int i = 0; i < (int)schema.grades.size(); i++)
          {
            auto& grade = schema.grades[i];
            ImGui::Text("%s: %i", grade.namePlural.c_str(), skillCheck.gradeCounts[i]);
          }

          ImGui::Text(strings.get(Strings::ArcadeAccuracyFormat).c_str(), skillCheck.accuracy_score_get(character));
          break;
        }

        case DUNGEON:
          break;

        case ORBIT:
          break;
      }
    };
    auto game_info_draw = [&](Game gameCurrent)
    {
      game_header_draw(gameCurrent);
      ImGui::Separator();
      game_info_sections_draw(gameCurrent);
      game_stats_draw(gameCurrent);
    };

    switch (state)
    {
      case GAMEPLAY:
        switch (game)
        {
          case SKILL_CHECK:
            if (skillCheck.update(resources, character, inventory, text, toasts))
            {
              game_reset(character, SKILL_CHECK);
              state = MENU;
            }
            break;

          case DUNGEON:
            if (dungeon.update(character))
            {
              game_reset(character, DUNGEON);
              state = MENU;
            }
            break;

          case ORBIT:
            if (orbit.update(resources, character, cursor, inventory, text, toasts))
            {
              game_reset(character, ORBIT);
              state = MENU;
            }
            break;
        }
        return;

      case MENU:
      case INFO:
        break;
    }

    auto buttonHeight = ImGui::GetFrameHeightWithSpacing();
    auto childSize = ImVec2(available.x, std::max(0.0f, available.y - buttonHeight));

    if (ImGui::BeginChild("##Arcade Child", childSize))
    {
      switch (state)
      {
        case MENU:
          game_menu_draw(ORBIT);
          //game_menu_draw(DUNGEON);
          game_menu_draw(SKILL_CHECK);
          break;

        case INFO:
          game_info_draw(game);
          break;

        case GAMEPLAY:
          break;
      }
    }
    ImGui::EndChild();

    if (state == INFO)
    {
      if (WIDGET_FX(ImGui::Button(strings.get(Strings::ArcadeBackButton).c_str()))) state = MENU;
    }
  }
}
