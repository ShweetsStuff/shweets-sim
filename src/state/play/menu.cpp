#include "menu.hpp"

#include "style.hpp"

#include "../../util/imgui.hpp"
#include "../../util/imgui/widget.hpp"

#include <algorithm>

using namespace game::util;
using namespace game::util::imgui;
using namespace game::resource::xml;

namespace game::state::play
{
  void Menu::tick()
  {
    inventory.tick();
    arcade.tick();
  }

  void Menu::update(Resources& resources, ItemManager& itemManager, entity::Character& character,
                    entity::Cursor& cursor, Text& text, Canvas& canvas)
  {
    static constexpr auto WIDTH_MULTIPLIER = 0.30f;

    auto& schema = character.data.menuSchema;
    auto& strings = character.data.strings;

    auto style = ImGui::GetStyle();
    auto& io = ImGui::GetIO();

    slide.update(isOpen, io.DeltaTime);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, style.FrameRounding);

    auto windowSize = imgui::to_ivec2(ImGui::GetMainViewport()->Size);

    auto size = ImVec2(windowSize.x * WIDTH_MULTIPLIER, windowSize.y - style.WindowPadding.y * 2);
    auto targetX = windowSize.x - size.x;
    auto t = slide.value_get();
    auto eased = slide.eased_get();
    auto posX = windowSize.x + (targetX - windowSize.x) * eased;

    auto pos = ImVec2(posX, style.WindowPadding.y);
    auto barSize = ImVec2(ImGui::GetTextLineHeightWithSpacing(), windowSize.y - style.WindowPadding.y * 2);
    auto barPos = ImVec2(pos.x - barSize.x - style.WindowPadding.x, style.WindowPadding.y);

    if (slide.is_visible())
    {
      ImGui::SetNextWindowSize(size);
      ImGui::SetNextWindowPos(pos);

      if (ImGui::Begin("##Menu", nullptr,
                       ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                           ImGuiWindowFlags_NoMove))
      {
        if (ImGui::BeginTabBar("##Options"))
        {

          if (WIDGET_FX(ImGui::BeginTabItem(strings.get(Strings::MenuTabInteract).c_str())))
          {
            interact.update(resources, text, character);
            ImGui::EndTabItem();
          }

          if (WIDGET_FX(ImGui::BeginTabItem(strings.get(Strings::MenuTabArcade).c_str())))
          {
            arcade.update(resources, character, cursor, inventory, text, toasts);
            ImGui::EndTabItem();
          }

          if (WIDGET_FX(ImGui::BeginTabItem(strings.get(Strings::MenuTabInventory).c_str())))
          {
            inventory.update(resources, itemManager, character);
            ImGui::EndTabItem();
          }

          if (WIDGET_FX(ImGui::BeginTabItem(strings.get(Strings::MenuTabSettings).c_str())))
          {
            settingsMenu.update(resources, SettingsMenu::PLAY, &strings);
            if (settingsMenu.isJustColorSet) style::color_set(resources, character);
            ImGui::EndTabItem();
          }

          if (isCheats && WIDGET_FX(ImGui::BeginTabItem(strings.get(Strings::MenuTabCheats).c_str())))
          {
            cheats.update(resources, character, inventory);
            ImGui::EndTabItem();
          }

#if DEBUG
          if (WIDGET_FX(ImGui::BeginTabItem("Debug")))
          {
            debug.update(character, cursor, itemManager, canvas, text);
            ImGui::EndTabItem();
          }
#endif
        }
        ImGui::EndTabBar();
      }
      ImGui::End();
    }

    ImGui::SetNextWindowSize(barSize);
    ImGui::SetNextWindowPos(barPos);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
    if (ImGui::Begin("##Menu Open Bar", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove))
    {
      auto buttonSize = ImGui::GetContentRegionAvail();
      auto cursorPos = ImGui::GetCursorScreenPos();

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding);
      auto result = WIDGET_FX(ImGui::Button("##MenuToggle", buttonSize));

      if (t <= 0.0f || t >= 1.0f)
      {
        ImGui::SetItemTooltip("%s", strings.get(isOpen ? Strings::MenuCloseTooltip : Strings::MenuOpenTooltip).c_str());
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
        tip = ImVec2(center.x + half, center.y);
        baseA = ImVec2(center.x - half, center.y - half);
        baseB = ImVec2(center.x - half, center.y + half);
      }
      else
      {
        tip = ImVec2(center.x - half, center.y);
        baseA = ImVec2(center.x + half, center.y - half);
        baseB = ImVec2(center.x + half, center.y + half);
      }

      auto color = ImGui::GetColorU32(ImGuiCol_Text);
      ImGui::GetWindowDrawList()->AddTriangleFilled(tip, baseA, baseB, color);
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleVar(2);
  }
}
