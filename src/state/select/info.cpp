#include "info.hpp"

#include "../../util/color.hpp"
#include "../../util/imgui.hpp"
#include "../../util/imgui/style.hpp"
#include "../../util/imgui/widget.hpp"
#include "../../util/vector.hpp"

using namespace game::util;
using namespace game::util::imgui;
using namespace game::util::measurement;
using namespace game::resource;

namespace game::state::select
{
  void Info::update(Resources& resources, int characterIndex)
  {
    if (!vector::in_bounds(resources.characterPreviews, characterIndex)) return;

    auto& style = ImGui::GetStyle();
    auto viewport = ImGui::GetMainViewport();
    auto& character = resources.characterPreviews[characterIndex];

    auto size = ImVec2(viewport->Size.x / 2.0f - (style.WindowPadding.x * 2.0f),
                       (viewport->Size.y / 2.0f) - (style.WindowPadding.y * 2.0f));
    auto pos = ImVec2(style.WindowPadding.x, (viewport->Size.y / 2.0f) + style.WindowPadding.y);

    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowPos(pos);
    imgui::style::color_set(character.color);

    if (ImGui::Begin("##Info", nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoTitleBar))
    {
      auto& save = character.save;
      auto& system = resources.settings.measurementSystem;
      auto& weight = save.is_valid() ? save.weight : character.weight;

      ImGui::PushFont(ImGui::GetFont(), Font::HEADER_3);

      auto childSize = imgui::size_without_footer_get();

      if (ImGui::BeginChild("##Info Child", childSize))
      {
        ImGui::PushFont(ImGui::GetFont(), Font::HEADER_3);
        ImGui::TextUnformatted(character.name.c_str());
        ImGui::PopFont();

        if (!character.description.empty())
        {
          ImGui::Separator();

          ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(imgui::to_imvec4(color::GRAY)));
          ImGui::PushFont(ImGui::GetFont(), Font::BIG);
          ImGui::TextWrapped("%s", character.description.c_str());
          ImGui::PopFont();

          ImGui::PopStyleColor();
        }

        ImGui::Separator();

        ImGui::PushFont(ImGui::GetFont(), Font::BIG);

        ImGui::Text("Weight: %0.2f %s", system == IMPERIAL ? weight * KG_TO_LB : weight,
                    system == IMPERIAL ? "lbs" : "kg");
        ImGui::Text("Stages: %i", character.stages);

        ImGui::Separator();

        ImGui::PopFont();

        ImGui::PushFont(ImGui::GetFont(), Font::NORMAL);

        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(imgui::to_imvec4(color::GRAY)));

        if (!character.author.empty()) ImGui::TextWrapped("Author: %s", character.author.c_str());

        ImGui::PopStyleColor();

        ImGui::PopFont();
      }
      ImGui::EndChild();

      auto widgetSize = row_widget_size_get(save.is_valid() ? 2 : 1);

      if (save.is_valid())
      {
        if (WIDGET_FX(ImGui::Button("Continue", widgetSize))) isContinue = true;
        ImGui::PushFont(ImGui::GetFont(), Font::NORMAL);
        ImGui::SetItemTooltip("%s", "Continue from a saved game.");
        ImGui::PopFont();
        ImGui::SameLine();
      }

      if (WIDGET_FX(ImGui::Button("New Game", widgetSize)))
      {
        if (save.is_valid())
        {
          ImGui::OpenPopup("New Game Warning");
          isNewGameWarning = true;
        }
        else
          isNewGame = true;
      }
      ImGui::PushFont(ImGui::GetFont(), Font::NORMAL);
      ImGui::SetItemTooltip("%s", "Start a new game.\nThis will delete progress!");
      ImGui::PopFont();

      ImGui::PopFont();

      ImGui::SetNextWindowSize(ImVec2(viewport->Size.x * 0.5f, 0), ImGuiCond_Always);
      ImGui::SetNextWindowPos(ImVec2(viewport->GetCenter()), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

      if (ImGui::BeginPopupModal("New Game Warning", &isNewGameWarning,
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
      {
        auto popupWidgetSize = row_widget_size_get(save.is_valid() ? 2 : 1);
        ImGui::TextWrapped("This will delete saved progress! Are you sure?");
        if (WIDGET_FX(ImGui::Button("Yes", popupWidgetSize))) isNewGame = true;
        ImGui::SameLine();
        if (WIDGET_FX(ImGui::Button("No", popupWidgetSize))) isNewGameWarning = false;
        ImGui::EndPopup();
      }
    }
    ImGui::End();
    imgui::style::color_set(resources.settings.color);
  }
}
