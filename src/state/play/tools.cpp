#include "tools.hpp"

#include "../../util/imgui.hpp"
#include "../../util/imgui/widget.hpp"

#include <algorithm>

using namespace game::util;
using namespace game::util::imgui;

namespace game::state::play
{
  void Tools::update(entity::Character& character, entity::Cursor& cursor, World& world, World::Focus focus,
                     Canvas& canvas)
  {
    static constexpr auto WIDTH_MULTIPLIER = 0.05f;

    auto style = ImGui::GetStyle();
    auto& io = ImGui::GetIO();
    auto& schema = character.data.menuSchema;

    slide.update(isOpen, io.DeltaTime);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, style.FrameRounding);

    auto windowSize = imgui::to_ivec2(ImGui::GetMainViewport()->Size);

    auto size = ImVec2(windowSize.x * WIDTH_MULTIPLIER, windowSize.y - style.WindowPadding.y * 2);
    auto targetX = 0;
    auto t = slide.value_get();
    auto eased = slide.eased_get();
    auto closedX = -size.x;
    auto posX = closedX + (targetX - closedX) * eased;
    auto pos = ImVec2(posX, style.WindowPadding.y);
    auto barSize = ImVec2(ImGui::GetTextLineHeightWithSpacing(), windowSize.y - style.WindowPadding.y * 2);
    auto barPos = ImVec2(pos.x + size.x, style.WindowPadding.y);

    if (slide.is_visible())
    {
      ImGui::SetNextWindowSize(size);
      ImGui::SetNextWindowPos(pos);

      if (ImGui::Begin("##Tools", nullptr,
                       ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                           ImGuiWindowFlags_NoMove))
      {
        auto buttonSize = imgui::to_imvec2(vec2(ImGui::GetContentRegionAvail().x));

        auto cursor_mode_button = [&](const std::string& name, int interactTypeID)
        {
          auto isMode = cursor.interactTypeID == interactTypeID;
          ImGui::PushStyleColor(ImGuiCol_Button,
                                ImGui::GetStyleColorVec4(isMode ? ImGuiCol_ButtonHovered : ImGuiCol_Button));
          if (WIDGET_FX(ImGui::Button(name.c_str(), buttonSize))) cursor.interactTypeID = interactTypeID;
          ImGui::PopStyleColor();
        };

        if (WIDGET_FX(ImGui::Button("Home", buttonSize))) world.character_focus(character, canvas, focus);
        ImGui::SetItemTooltip("%s", "Reset camera view.\n(Shortcut: Home)");

        for (int i = 0; i < (int)character.data.interactTypeNames.size(); i++)
          cursor_mode_button(character.data.interactTypeNames[i], i);
      }
      ImGui::End();
    }

    ImGui::SetNextWindowSize(barSize);
    ImGui::SetNextWindowPos(barPos);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
    if (ImGui::Begin("##Tools Open Bar", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove))
    {
      auto buttonSize = ImGui::GetContentRegionAvail();
      auto cursorPos = ImGui::GetCursorScreenPos();

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding);
      auto result = WIDGET_FX(ImGui::Button("##ToolsToggle", buttonSize));

      if (t <= 0.0f || t >= 1.0f)
      {
        ImGui::SetItemTooltip(isOpen ? "Close Tools" : "Open Tools");
        if (result)
        {
          isOpen = !isOpen;
          if (isOpen)
            schema.sounds.open.play();
          else
            schema.sounds.close.play();
        }
        if (!isOpen && t <= 0.0f && ImGui::IsItemHovered())
        {
          isOpen = true;
          schema.sounds.open.play();
        }
      }
      ImGui::PopStyleVar();

      auto center = ImVec2(cursorPos.x + (buttonSize.x * 0.5f), cursorPos.y + (buttonSize.y * 0.5f));
      auto half = std::min(buttonSize.x, buttonSize.y) * 0.22f;
      ImVec2 tip;
      ImVec2 baseA;
      ImVec2 baseB;
      if (isOpen)
      {
        tip = ImVec2(center.x - half, center.y);
        baseA = ImVec2(center.x + half, center.y - half);
        baseB = ImVec2(center.x + half, center.y + half);
      }
      else
      {
        tip = ImVec2(center.x + half, center.y);
        baseA = ImVec2(center.x - half, center.y - half);
        baseB = ImVec2(center.x - half, center.y + half);
      }

      auto color = ImGui::GetColorU32(ImGuiCol_Text);
      ImGui::GetWindowDrawList()->AddTriangleFilled(tip, baseA, baseB, color);
    }
    ImGui::End();
    ImGui::PopStyleVar(2);

    ImGui::PopStyleVar(2);
  }
}
