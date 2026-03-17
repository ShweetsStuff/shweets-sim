#include "text.hpp"

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <string_view>

#include "../../util/imgui.hpp"
#include "../../util/imgui/widget.hpp"
#include "../../util/math.hpp"

using namespace game::util;

namespace game::state::play
{
  const char* utf8_advance_chars(const char* text, const char* end, int count)
  {
    const char* it = text;
    while (it < end && count > 0)
    {
      unsigned int codepoint = 0;
      int step = ImTextCharFromUtf8(&codepoint, it, end);
      if (step <= 0) break;
      it += step;
      --count;
    }
    return it;
  }

  void Text::set(resource::xml::Dialogue::Entry* dialogueEntry, entity::Character& character, bool isInterruptible)
  {
    if (!dialogueEntry) return;
    this->entry = dialogueEntry;

    isFinished = false;
    index = 0;
    time = 0.0f;
    isEnabled = true;
    character.isTalking = true;
    if (!dialogueEntry->animation.empty())
      character.queue_play({.animation = dialogueEntry->animation, .isInterruptible = isInterruptible});
    if (dialogueEntry->text.empty()) isEnabled = false;
  }

  void Text::tick(entity::Character& character)
  {
    if (!entry || isFinished) return;

    index++;

    if (index >= ImTextCountCharsFromUtf8(entry->text.c_str(), entry->text.c_str() + entry->text.size()))
    {
      isFinished = true;
      character.isTalking = false;
    }
  }

  void Text::update(entity::Character& character)
  {
    static constexpr auto WIDTH_MULTIPLIER = 0.30f;
    static constexpr auto HEIGHT_MULTIPLIER = 6.0f;

    if (!entry) return;
    auto& dialogue = character.data.dialogue;
    auto& menuSchema = character.data.menuSchema;

    auto& style = ImGui::GetStyle();
    auto windowSize = imgui::to_ivec2(ImGui::GetMainViewport()->Size);

    auto size = ImVec2(windowSize.x * WIDTH_MULTIPLIER - (style.WindowPadding.x * 2.0f),
                       ImGui::GetTextLineHeightWithSpacing() * HEIGHT_MULTIPLIER);
    auto pos = ImVec2((windowSize.x * 0.5f) - (size.x * 0.5f), windowSize.y - size.y - style.WindowPadding.y);

    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowPos(pos);

    if (!entry) return;

    if (ImGui::Begin("##Text", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove))
    {
      auto isHovered = ImGui::IsWindowHovered();
      auto isMouse = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
      auto isSpace = ImGui::IsKeyReleased(ImGuiKey_Space);
      auto isAdvance = (isHovered && (isMouse || isSpace));

      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, 0));

      if (ImGui::BeginTabBar("##Name"))
      {
        if (ImGui::BeginTabItem(character.data.name.c_str())) ImGui::EndTabItem();
        ImGui::EndTabBar();
      }

      auto available = ImGui::GetContentRegionAvail();

      auto font = ImGui::GetFont();
      auto fontSize = resource::Font::NORMAL;

      ImGui::PushFont(font, (float)fontSize);

      auto text = [&]()
      {
        auto text = entry ? std::string_view(entry->text) : "null";
        auto length = std::clamp(index, 0, ImTextCountCharsFromUtf8(text.data(), text.data() + text.size()));

        if (length <= 0)
        {
          ImGui::Dummy(ImVec2(1.0f, ImGui::GetTextLineHeight()));
          return;
        }

        const char* textStart = text.data();
        const char* textEnd = textStart + text.size();
        const char* textLimit = utf8_advance_chars(textStart, textEnd, length);

        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + available.x);
        ImGui::TextUnformatted(textStart, textLimit);
        ImGui::PopTextWrapPos();
      };

      text();

      if (entry)
      {
        if (isFinished)
        {
          if (!entry->choices.empty())
          {
            ImGui::SetCursorPos(ImVec2(ImGui::GetStyle().WindowPadding.x, available.y));
            auto buttonSize = imgui::row_widget_size_get((int)entry->choices.size());

            for (auto& branch : entry->choices)
            {
              if (WIDGET_FX(ImGui::Button(branch.text.c_str(), buttonSize)))
                set(dialogue.get(branch.nextID), character);

              ImGui::SetItemTooltip("%s", branch.text.c_str());
              ImGui::SameLine();
            }

            if (isHovered && isSpace)
            {
              set(dialogue.get(entry->choices.front().nextID), character);
              menuSchema.sounds.select.play();
            }
          }
          else
          {
            if (entry->nextID != -1)
            {
              ImGui::SetCursorPos(ImVec2(available.x - ImGui::GetTextLineHeightWithSpacing(), available.y));
              auto indicatorSize = ImVec2(ImGui::GetTextLineHeightWithSpacing(), ImGui::GetTextLineHeightWithSpacing());
              auto cursorPos = ImGui::GetCursorScreenPos();
              auto center = ImVec2(cursorPos.x + (indicatorSize.x * 0.5f), cursorPos.y + (indicatorSize.y * 0.5f));
              auto half = std::min(indicatorSize.x, indicatorSize.y) * 0.35f;
              auto tip = ImVec2(center.x + half, center.y);
              auto baseA = ImVec2(center.x - half, center.y - half);
              auto baseB = ImVec2(center.x - half, center.y + half);
              auto color = ImGui::GetColorU32(ImGuiCol_Text);
              ImGui::GetWindowDrawList()->AddTriangleFilled(tip, baseA, baseB, color);
              ImGui::Dummy(indicatorSize);

              if (isAdvance)
              {
                menuSchema.sounds.select.play();
                set(dialogue.get(entry->nextID), character);
              }
            }
            else if (isAdvance)
            {
              isEnabled = false;
              entry = nullptr;
            }
          }
        }
        else
        {
          if (isAdvance)
          {
            index = ImTextCountCharsFromUtf8(entry->text.c_str(), entry->text.c_str() + entry->text.size());
            isFinished = true;
            character.isTalking = false;
          }
        }
      }

      ImGui::PopFont();
      ImGui::PopStyleVar();
    };
    ImGui::End();

    if (isEnabled && isFinished && entry && entry->is_last())
    {
      if (time += ImGui::GetIO().DeltaTime; time > LIFETIME)
      {
        isEnabled = false;
        entry = nullptr;
      }
    }
  }

  bool Text::is_interruptible() const { return !entry || (entry && entry->is_last()); }
}
