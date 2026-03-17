#include "toasts.hpp"

#include <imgui.h>
#include <ranges>

namespace game::state::play
{
  void Toasts::tick()
  {
    for (int i = 0; i < (int)items.size(); i++)
    {
      auto& item = items[i];

      item.lifetime--;

      if (item.lifetime <= 0)
      {
        items.erase(items.begin() + i--);
        continue;
      }
    }
  }

  void Toasts::update()
  {
    if (items.empty()) return;

    auto viewport = ImGui::GetMainViewport();
    auto& style = ImGui::GetStyle();

    auto windowBgColor = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
    auto borderColor = ImGui::GetStyleColorVec4(ImGuiCol_Border);
    auto textColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);

    for (int i = 0; i < (int)items.size(); i++)
    {
      auto& item = items[i];
      auto posY = viewport->Size.y - style.WindowPadding.y -
                  (((ImGui::GetTextLineHeightWithSpacing() + style.WindowPadding.y * 2)) * (items.size() - i));

      ImGui::SetNextWindowPos(ImVec2(style.WindowPadding.x, posY));

      ImGui::SetNextWindowSize(ImVec2(ImGui::CalcTextSize(item.message.c_str()).x + (style.WindowPadding.x * 2),
                                      ImGui::GetTextLineHeightWithSpacing()));

      auto alpha = (float)item.lifetime / Item::LIFETIME;

      windowBgColor.w = alpha;
      borderColor.w = alpha;
      textColor.w = alpha;

      ImGui::PushStyleColor(ImGuiCol_WindowBg, windowBgColor);
      ImGui::PushStyleColor(ImGuiCol_Border, borderColor);
      ImGui::PushStyleColor(ImGuiCol_Text, textColor);

      auto name = "##Toast " + std::to_string(i);
      if (ImGui::Begin(name.c_str(), nullptr,
                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_NoScrollbar))
        ImGui::TextUnformatted(item.message.c_str());
      ImGui::End();

      ImGui::PopStyleColor(3);
    }
  }

  void Toasts::push(const std::string& message) { items.push_back({message, Item::LIFETIME}); }
}
