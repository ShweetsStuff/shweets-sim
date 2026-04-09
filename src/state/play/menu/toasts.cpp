#include "toasts.hpp"

namespace game::state::play::menu
{
  namespace
  {
    static constexpr auto TOAST_MESSAGE_SPEED = 1.0f;
  }

  void Toasts::spawn(const std::string& message, const ImVec2& position, int time)
  {
    toasts.emplace_back(message, position, time, time);
  }

  void Toasts::update(ImDrawList* drawList)
  {
    if (!drawList) return;

    for (int i = 0; i < (int)toasts.size(); i++)
    {
      auto& toast = toasts[i];
      toast.position.y -= TOAST_MESSAGE_SPEED;
      auto textColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
      textColor.w = (float)toast.time / toast.timeMax;
      drawList->AddText(toast.position, ImGui::GetColorU32(textColor), toast.message.c_str());

      toast.time--;
      if (toast.time <= 0) toasts.erase(toasts.begin() + i--);
    }
  }
}
