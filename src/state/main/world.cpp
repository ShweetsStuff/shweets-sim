#include "world.hpp"

#include "../../util/imgui.hpp"
#include "../../util/math.hpp"

#include <algorithm>
#include <cmath>

using namespace game::util;

namespace game::state::main
{
  void World::set(entity::Character& character, Canvas& canvas, Focus focus)
  {
    character.stage = character.stage_get();
    character_focus(character, canvas, focus);
  }

  void World::update(entity::Character& character, entity::Cursor& cursor, Canvas& canvas, Focus focus)
  {
    auto& cursorSchema = character.data.cursorSchema;
    auto& pan = canvas.pan;
    auto& zoom = canvas.zoom;
    auto& io = ImGui::GetIO();
    auto isMouseMiddleDown = ImGui::IsMouseDown(ImGuiMouseButton_Middle);
    auto isMouseLeftDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    auto isCtrlDown = ImGui::IsKeyDown(ImGuiMod_Ctrl);
    auto panMultiplier = ZOOM_BASE / zoom;

    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemActive())
    {
      if ((isMouseMiddleDown) || (isMouseLeftDown && isCtrlDown))
      {
        if (auto animation = cursorSchema.animations.pan.get()) cursor.queue_play({*animation});
        pan -= imgui::to_vec2(io.MouseDelta) * panMultiplier;
      }

      if (io.MouseWheel != 0)
      {
        auto viewport = ImGui::GetMainViewport();
        auto mousePos = io.MousePos;
        auto cursorPos = imgui::to_vec2(ImVec2(mousePos.x - viewport->Pos.x, mousePos.y - viewport->Pos.y));
        auto zoomBefore = zoom;
        auto zoomFactorBefore = math::to_unit(zoomBefore);
        auto cursorWorld = pan + (cursorPos / zoomFactorBefore);

        if (auto animation = cursorSchema.animations.zoom.get()) cursor.queue_play({*animation});

        zoom = glm::clamp(ZOOM_MIN, zoom + (io.MouseWheel * ZOOM_STEP), ZOOM_MAX);

        auto zoomFactorAfter = math::to_unit(zoom);
        pan = cursorWorld - (cursorPos / zoomFactorAfter);
      }
    }

    zoom = glm::clamp(ZOOM_MIN, zoom, ZOOM_MAX);

    if (ImGui::IsKeyPressed(ImGuiKey_Home)) character_focus(character, canvas, focus);
  }

  void World::character_focus(entity::Character& character, Canvas& canvas, Focus focus)
  {
    static constexpr float MENU_WIDTH_MULTIPLIER = 0.30f;
    static constexpr float TOOLS_WIDTH_MULTIPLIER = 0.10f;
    static constexpr float PADDING = 100.0f;

    auto rect = character.rect();

    if (!std::isfinite(rect.x) || !std::isfinite(rect.y) || !std::isfinite(rect.z) || !std::isfinite(rect.w) ||
        rect.z <= 0.0f || rect.w <= 0.0f)
      return;

    rect = {rect.x - PADDING * 0.5f, rect.y - PADDING * 0.5f, rect.z + PADDING, rect.w + PADDING};

    auto zoomFactor = std::min((float)canvas.size.x / rect.z, (float)canvas.size.y / rect.w);
    canvas.zoom = glm::clamp(ZOOM_MIN, math::to_percent(zoomFactor), ZOOM_MAX);
    zoomFactor = math::to_unit(canvas.zoom);

    auto rectCenter = glm::vec2(rect.x + rect.z * 0.5f, rect.y + rect.w * 0.5f);
    auto viewSizeWorld = glm::vec2(canvas.size) / zoomFactor;
    canvas.pan = rectCenter - (vec2(viewSizeWorld.x, viewSizeWorld.y) * 0.5f);
    auto menuWidthWorld = (canvas.size.x * MENU_WIDTH_MULTIPLIER) / zoomFactor;
    auto toolsWidthWorld = (canvas.size.x * TOOLS_WIDTH_MULTIPLIER) / zoomFactor;

    if (focus == Focus::MENU || focus == Focus::MENU_TOOLS) canvas.pan.x += menuWidthWorld * 0.5f;
    if (focus == Focus::TOOLS || focus == Focus::MENU_TOOLS) canvas.pan.x -= toolsWidthWorld * 0.5f;

    auto panMin = glm::vec2(0.0f, 0.0f);
    auto panMax = glm::max(glm::vec2(0.0f), SIZE - viewSizeWorld);
    canvas.pan = glm::clamp(panMin, canvas.pan, panMax);
  }
}
