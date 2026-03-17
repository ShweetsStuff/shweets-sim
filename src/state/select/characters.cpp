#include "characters.hpp"

#include "../../util/imgui/style.hpp"
#include "../../util/imgui/widget.hpp"

using namespace game::util;

namespace game::state::select
{
  void Characters::update(Resources& resources, int& characterIndex)
  {
    auto& style = ImGui::GetStyle();
    auto viewport = ImGui::GetMainViewport();

    auto size =
        ImVec2(viewport->Size.x / 2.0f - style.WindowPadding.x, viewport->Size.y - (style.WindowPadding.y * 2.0f));
    auto pos = ImVec2(viewport->Size.x / 2.0f, style.WindowPadding.y);

    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowPos(pos);

    if (ImGui::Begin("##Main Menu", nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoTitleBar))
    {
      if (ImGui::BeginTabBar("##Main Menu Bar"))
      {
        if (WIDGET_FX(ImGui::BeginTabItem("Characters")))
        {
          auto cursorPos = ImGui::GetCursorPos();
          auto cursorStartX = ImGui::GetCursorPosX();

          auto buttonSize = ImVec2(ImGui::GetContentRegionAvail().x / 4, ImGui::GetContentRegionAvail().x / 4);

          for (int i = 0; i < (int)resources.characterPreviews.size(); i++)
          {
            auto& character = resources.characterPreviews[i];
            ImGui::PushID(i);

            ImGui::SetCursorPos(cursorPos);
            imgui::style::color_set(character.color);

            auto isSelected = i == characterIndex;

            if (isSelected) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered));

            if (character.portrait.is_valid())
            {
              if (WIDGET_FX(ImGui::ImageButton(character.name.c_str(), character.portrait.id, buttonSize)))
                characterIndex = i;
            }
            else if (WIDGET_FX(ImGui::Button(character.name.c_str(), buttonSize)))
              characterIndex = i;
            if (isSelected) ImGui::PopStyleColor();
            ImGui::SetItemTooltip("%s", character.name.c_str());

            auto increment = ImGui::GetItemRectSize().x + ImGui::GetStyle().ItemSpacing.x;
            cursorPos.x += increment;

            if (cursorPos.x + increment > ImGui::GetContentRegionAvail().x)
            {
              cursorPos.x = cursorStartX;
              cursorPos.y += increment;
            }

            ImGui::PopID();
            imgui::style::color_set(resources.settings.color);
          }
          ImGui::EndTabItem();
        }

        if (WIDGET_FX(ImGui::BeginTabItem("Settings")))
        {
          settingsMenu.update(resources);
          ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
      }
      ImGui::End();
    }
  }
}
