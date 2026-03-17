#include "settings_menu.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "../util/imgui/style.hpp"
#include "../util/imgui/widget.hpp"
#include "../util/math.hpp"
#include "../util/measurement.hpp"

using namespace game::util;
using namespace game::util::imgui;

namespace game::state
{
  void SettingsMenu::update(Resources& resources, Mode mode)
  {
    auto& settings = resources.settings;
    auto& measurementSystem = settings.measurementSystem;
    auto& volume = settings.volume;
    auto& color = settings.color;

    isJustColorSet = false;

    ImGui::SeparatorText("Measurement System");
    WIDGET_FX(ImGui::RadioButton("Metric", (int*)&measurementSystem, measurement::METRIC));
    ImGui::SetItemTooltip("%s", "Use kilograms (kg).");
    ImGui::SameLine();
    WIDGET_FX(ImGui::RadioButton("Imperial", (int*)&measurementSystem, measurement::IMPERIAL));
    ImGui::SetItemTooltip("%s", "Use pounds (lbs).");

    ImGui::SeparatorText("Sound");
    if (WIDGET_FX(ImGui::SliderInt("Volume", &volume, 0, 100, "%d%%")))
      resources.volume_set(math::to_unit((float)volume));
    ImGui::SetItemTooltip("%s", "Adjust master volume.");

    ImGui::SeparatorText("Appearance");

    if (WIDGET_FX(ImGui::Checkbox("Use Character Color", &settings.isUseCharacterColor))) isJustColorSet = true;
    ImGui::SetItemTooltip("When playing, the UI will use the character's preset UI color.");
    ImGui::SameLine();
    ImGui::BeginDisabled(settings.isUseCharacterColor);
    if (WIDGET_FX(
            ImGui::ColorEdit3("Color", value_ptr(color), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip)))
    {
      style::color_set(color);
      isJustColorSet = true;
    }
    ImGui::SetItemTooltip("%s", "Change the UI color.");
    ImGui::EndDisabled();

    ImGui::Separator();
    if (WIDGET_FX(ImGui::Button("Reset to Default", ImVec2(-FLT_MIN, 0))))
    {
      settings = resource::xml::Settings();
      style::color_set(settings.color);
    }

    if (mode == PLAY)
    {
      ImGui::Separator();

      if (WIDGET_FX(ImGui::Button("Save", ImVec2(-FLT_MIN, 0)))) isSave = true;
      ImGui::SetItemTooltip("%s", "Save the game.\n(Note: the game autosaves frequently.)");

      if (WIDGET_FX(ImGui::Button("Return to Characters", ImVec2(-FLT_MIN, 0)))) isGoToSelect = true;
      ImGui::SetItemTooltip("%s", "Go back to the character selection screen.\nProgress will be saved.");
    }
  }
}
