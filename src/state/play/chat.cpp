#include "chat.hpp"

#include "../../util/imgui/widget.hpp"

using namespace game::resource;
using namespace game::util::imgui;

namespace game::state::play
{
  void Chat::update(Resources&, Text& text, entity::Character& character)
  {
    auto& dialogue = character.data.dialogue;
    auto size = ImGui::GetContentRegionAvail();

    ImGui::PushFont(ImGui::GetFont(), Font::HEADER_2);

    if (dialogue.random.is_valid())
      if (WIDGET_FX(ImGui::Button("Let's chat!", ImVec2(size.x, 0))))
        text.set(dialogue.get(dialogue.random), character);

    ImGui::PopFont();

    if (dialogue.help.is_valid())
      if (WIDGET_FX(ImGui::Button("Help", ImVec2(size.x, 0)))) text.set(dialogue.get(dialogue.help), character);

    auto stage = glm::clamp(0, character.stage_get(), character.stage_max_get());
    auto& pool = stage > 0 ? character.data.stages.at(stage - 1).pool : character.data.pool;

    if (pool.is_valid())
      if (WIDGET_FX(ImGui::Button("How are you feeling?", ImVec2(size.x, 0)))) text.set(dialogue.get(pool), character);
  }
}
