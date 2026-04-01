#include "settings_menu.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "../util/imgui/style.hpp"
#include "../util/imgui/widget.hpp"
#include "../util/math.hpp"
#include "../util/measurement.hpp"

using namespace game::util;
using namespace game::util::imgui;
using namespace game::resource::xml;

namespace game::state
{
  void SettingsMenu::update(Resources& resources, Mode mode, const Strings* strings)
  {
    auto& settings = resources.settings;
    auto& measurementSystem = settings.measurementSystem;
    auto& volume = settings.volume;
    auto& color = settings.color;
    auto string_get = [&](Strings::Type type, const char* fallback) -> const char*
    {
      return strings ? strings->get(type).c_str() : fallback;
    };

    isJustColorSet = false;

    ImGui::SeparatorText(string_get(Strings::SettingsMeasurementSystem, "Measurement System"));
    WIDGET_FX(ImGui::RadioButton(string_get(Strings::SettingsMetric, "Metric"),
                                 (int*)&measurementSystem, measurement::METRIC));
    ImGui::SetItemTooltip("%s", string_get(Strings::SettingsMetricTooltip, "Use kilograms (kg)."));
    ImGui::SameLine();
    WIDGET_FX(ImGui::RadioButton(string_get(Strings::SettingsImperial, "Imperial"),
                                 (int*)&measurementSystem, measurement::IMPERIAL));
    ImGui::SetItemTooltip("%s", string_get(Strings::SettingsImperialTooltip, "Use pounds (lbs)."));

    ImGui::SeparatorText(string_get(Strings::SettingsSound, "Sound"));
    if (WIDGET_FX(
            ImGui::SliderInt(string_get(Strings::SettingsVolume, "Volume"), &volume, 0, 100, "%d%%")))
      resources.volume_set(math::to_unit((float)volume));
    ImGui::SetItemTooltip("%s", string_get(Strings::SettingsVolumeTooltip, "Adjust master volume."));

    ImGui::SeparatorText(string_get(Strings::SettingsAppearance, "Appearance"));

    if (WIDGET_FX(ImGui::Checkbox(string_get(Strings::SettingsUseCharacterColor,
                                             "Use Character Color"),
                                  &settings.isUseCharacterColor)))
      isJustColorSet = true;
    ImGui::SetItemTooltip("%s", string_get(Strings::SettingsUseCharacterColorTooltip,
                                           "When playing, the UI will use the character's preset UI color."));
    ImGui::SameLine();
    ImGui::BeginDisabled(settings.isUseCharacterColor);
    if (WIDGET_FX(
            ImGui::ColorEdit3(string_get(Strings::SettingsColor, "Color"), value_ptr(color),
                              ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip)))
    {
      style::color_set(color);
      isJustColorSet = true;
    }
    ImGui::SetItemTooltip("%s", string_get(Strings::SettingsColorTooltip, "Change the UI color."));
    ImGui::EndDisabled();

    ImGui::Separator();
    if (WIDGET_FX(ImGui::Button(string_get(Strings::SettingsResetButton, "Reset to Default"),
                                ImVec2(-FLT_MIN, 0))))
    {
      settings = resource::xml::Settings();
      style::color_set(settings.color);
    }

    if (mode == PLAY)
    {
      ImGui::Separator();

      if (WIDGET_FX(
              ImGui::Button(string_get(Strings::SettingsSaveButton, "Save"), ImVec2(-FLT_MIN, 0))))
        isSave = true;
      ImGui::SetItemTooltip(
          "%s", string_get(Strings::SettingsSaveTooltip,
                           "Save the game.\n(Note: the game autosaves frequently.)"));

      if (WIDGET_FX(ImGui::Button(
              string_get(Strings::SettingsReturnToCharactersButton, "Return to Characters"),
              ImVec2(-FLT_MIN, 0))))
        isGoToSelect = true;
      ImGui::SetItemTooltip("%s", string_get(Strings::SettingsReturnToCharactersTooltip,
                                             "Go back to the character selection screen.\nProgress will be saved."));
    }
  }
}
