#include "preview.hpp"

#include <algorithm>
#include <cmath>

#include "../../util/imgui.hpp"
#include "../../util/imgui/style.hpp"
#include "../../util/imgui/widget.hpp"
#include "../../util/vector.hpp"

using namespace game::entity;
using namespace game::resource;
using namespace game::util;
using namespace game::util::imgui;

namespace game::state::select
{
  void Preview::tick(int characterIndex)
  {
    if (characterIndex != -1 && isInGame) actor.tick();
  }

  void Preview::update(Resources& resources, int characterIndex)
  {
    if (!vector::in_bounds(resources.characterPreviews, characterIndex)) return;

    auto& style = ImGui::GetStyle();
    auto viewport = ImGui::GetMainViewport();
    auto& character = resources.characterPreviews[characterIndex];
    auto size = ImVec2(viewport->Size.x / 2.0f - (style.WindowPadding.x * 2.0f),
                       (viewport->Size.y / 2.0f) - (style.WindowPadding.y * 2.0f));
    auto pos = ImVec2(style.WindowPadding.x, style.WindowPadding.y);

    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowPos(pos);
    imgui::style::color_set(character.color);

    if (ImGui::Begin("##Preview", nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoTitleBar))
    {
      if (ImGui::BeginTabBar("##Preview Tab Bar"))
      {
        auto available = ImGui::GetContentRegionAvail();
        auto availableSize = imgui::to_vec2(available);
        auto textureSize = vec2(character.render.size);

        if (WIDGET_FX(ImGui::BeginTabItem("Render")))
        {
          auto scale =
              (availableSize.x <= 0.0f || availableSize.y <= 0.0f || textureSize.x <= 0.0f || textureSize.y <= 0.0f)
                  ? 0.0f
                  : std::min(availableSize.x / textureSize.x, availableSize.y / textureSize.y);

          auto renderSize = ImVec2(textureSize.x * scale, textureSize.y * scale);

          ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + (availableSize.x * 0.5f) - (renderSize.y * 0.5f),
                                     ImGui::GetCursorPosY() + (availableSize.y * 0.5f) - (renderSize.y * 0.5f)));

          ImGui::Image(character.render.id, renderSize);

          ImGui::EndTabItem();
        }

        if (WIDGET_FX(ImGui::BeginTabItem("In Game")))
        {
          isInGame = true;

          if (previousCharacterIndex != characterIndex)
          {
            actor = Actor(resources.characterPreviews[characterIndex].anm2);
            rect = actor.rect();
            previousCharacterIndex = characterIndex;
          }

          auto rectSize = vec2(rect.z, rect.w);
          auto previewScale = (availableSize.x <= 0.0f || availableSize.y <= 0.0f || rectSize.x <= 0.0f ||
                               rectSize.y <= 0.0f || !std::isfinite(rectSize.x) || !std::isfinite(rectSize.y))
                                  ? 0.0f
                                  : std::min(availableSize.x / rectSize.x, availableSize.y / rectSize.y);

          auto previewSize = rectSize * previewScale;
          auto canvasSize = ivec2(std::max(1.0f, previewSize.x), std::max(1.0f, previewSize.y));

          canvas.zoom = previewScale * 100.0f;
          canvas.pan = vec2(rect.x, rect.y);

          auto cursorPos = ImGui::GetCursorPos();
          ImGui::SetCursorPos(ImVec2(cursorPos.x + (availableSize.x * 0.5f) - ((float)canvasSize.x * 0.5f),
                                     cursorPos.y + (availableSize.y * 0.5f) - ((float)canvasSize.y * 0.5f)));

          canvas.bind();
          canvas.size_set(canvasSize);
          canvas.clear();
          actor.render(resources.shaders[shader::TEXTURE], resources.shaders[shader::RECT], canvas);
          canvas.unbind();

          ImGui::Image(canvas.texture, imgui::to_imvec2(canvasSize));

          ImGui::EndTabItem();
        }
        else
          isInGame = false;

        ImGui::EndTabBar();
      }
    }
    ImGui::End();
    imgui::style::color_set(resources.settings.color);
  }
}
