#include "style.hpp"

#include "../imgui.hpp"

namespace game::util::imgui::style
{
  void widget_set(float rounding)
  {
    constexpr auto SCROLLBAR_SIZE = 30.0f;

    auto& style = ImGui::GetStyle();
    style.WindowRounding = rounding;
    style.FrameRounding = rounding;
    style.GrabRounding = rounding;
    style.ScrollbarSize = SCROLLBAR_SIZE;
  }

  void color_set(glm::vec3 color)
  {
    static constexpr auto COLOR_BG_MULTIPLIER = 0.25f;
    static constexpr auto COLOR_BG_ALPHA = 0.90f;
    static constexpr auto COLOR_ACTIVE_MULTIPLIER = 1.30f;
    static constexpr auto COLOR_HOVERED_MULTIPLIER = 1.60f;
    static constexpr auto COLOR_ACTIVE_HOVERED_MIN = 0.1f;
    static constexpr auto COLOR_ACTIVE_HOVERED_MAX = 0.9f;
    static constexpr auto COLOR_ACCENT_MULTIPLIER = 2.0f;
    static constexpr auto COLOR_ACCENT_ACTIVE_MULTIPLIER = 2.25f;

    auto& colors = ImGui::GetStyle().Colors;
    auto active_hovered_clamp = [](glm::vec3 color)
    { return glm::clamp(color, glm::vec3(COLOR_ACTIVE_HOVERED_MIN), glm::vec3(COLOR_ACTIVE_HOVERED_MAX)); };

    auto colorNew = to_imvec4(glm::vec4(color, 1.0f));
    auto colorBg = to_imvec4(glm::vec4(color * COLOR_BG_MULTIPLIER, COLOR_BG_ALPHA));
    auto colorChildBg = to_imvec4(glm::vec4(color * COLOR_BG_MULTIPLIER, 0.0f));
    auto colorActive = to_imvec4(glm::vec4(active_hovered_clamp(color * COLOR_ACTIVE_MULTIPLIER), 1.0f));
    auto colorHovered = to_imvec4(glm::vec4(active_hovered_clamp(color * COLOR_HOVERED_MULTIPLIER), 1.0f));
    auto colorAccent = to_imvec4(glm::vec4(color * COLOR_ACCENT_MULTIPLIER, 1.0f));
    auto colorAccentActive = to_imvec4(glm::vec4(color * COLOR_ACCENT_ACTIVE_MULTIPLIER, 1.0f));

    colors[ImGuiCol_Button] = colorNew;
    colors[ImGuiCol_FrameBg] = colorNew;
    colors[ImGuiCol_FrameBgHovered] = colorHovered;
    colors[ImGuiCol_FrameBgActive] = colorActive;
    colors[ImGuiCol_Header] = colorNew;
    colors[ImGuiCol_HeaderHovered] = colorHovered;
    colors[ImGuiCol_HeaderActive] = colorActive;
    colors[ImGuiCol_Tab] = colorNew;
    colors[ImGuiCol_TitleBg] = colorNew;
    colors[ImGuiCol_TitleBgCollapsed] = colorNew;
    colors[ImGuiCol_TitleBgActive] = colorNew;
    colors[ImGuiCol_WindowBg] = colorBg;
    colors[ImGuiCol_ChildBg] = colorChildBg;
    colors[ImGuiCol_PopupBg] = colorBg;

    colors[ImGuiCol_ScrollbarBg] = colorNew;
    colors[ImGuiCol_ScrollbarGrab] = colorHovered;
    colors[ImGuiCol_ScrollbarGrabHovered] = colorHovered;
    colors[ImGuiCol_ScrollbarGrabActive] = colorAccent;

    colors[ImGuiCol_ButtonHovered] = colorHovered;
    colors[ImGuiCol_FrameBgHovered] = colorHovered;
    colors[ImGuiCol_TabHovered] = colorHovered;

    colors[ImGuiCol_ButtonActive] = colorActive;
    colors[ImGuiCol_FrameBgActive] = colorActive;
    colors[ImGuiCol_TabActive] = colorActive;

    colors[ImGuiCol_PlotHistogram] = colorAccent;
    colors[ImGuiCol_CheckMark] = colorAccent;
    colors[ImGuiCol_SliderGrab] = colorAccent;
    colors[ImGuiCol_SliderGrabActive] = colorAccentActive;
  }
}
