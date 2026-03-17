#include "skill_check.hpp"

#include <imgui_internal.h>

#include "../../../util/imgui.hpp"
#include "../../../util/imgui/widget.hpp"
#include "../../../util/math.hpp"

#include <cmath>
#include <format>
#include <ranges>

using namespace game::util;
using namespace game::entity;
using namespace game::resource;
using namespace glm;

namespace game::state::play
{
  float SkillCheck::accuracy_score_get(entity::Character& character)
  {
    if (totalPlays == 0) return 0.0f;

    auto& schema = character.data.skillCheckSchema;

    float combinedWeight{};

    for (int i = 0; i < (int)schema.grades.size(); i++)
    {
      auto& grade = schema.grades[i];
      combinedWeight += gradeCounts[i] * grade.weight;
    }

    return glm::clamp(0.0f, math::to_percent(combinedWeight / totalPlays), 100.0f);
  }

  SkillCheck::Challenge SkillCheck::challenge_generate(entity::Character& character)
  {
    auto& schema = character.data.skillCheckSchema;

    Challenge newChallenge;

    Range newRange{};

    auto rangeSize = std::max(schema.rangeMin, schema.rangeBase - (schema.rangeScoreBonus * score));
    newRange.min = math::random_max(1.0f - rangeSize);
    newRange.max = newRange.min + rangeSize;

    newChallenge.range = newRange;
    newChallenge.tryValue = 0.0f;

    newChallenge.speed =
        glm::clamp(schema.speedMin, schema.speedMin + (schema.speedScoreBonus * score), schema.speedMax);

    if (math::random_bool())
    {
      newChallenge.tryValue = 1.0f;
      newChallenge.speed *= -1;
    }

    return newChallenge;
  }

  SkillCheck::SkillCheck(entity::Character& character) { challenge = challenge_generate(character); }

  void SkillCheck::tick()
  {
    for (auto& [i, actor] : itemActors)
      actor.tick();
  }

  void SkillCheck::update(Resources& resources, entity::Character& character, Inventory& inventory, Text& text)
  {
    static constexpr auto BG_COLOR_MULTIPLIER = 0.5f;
    static constexpr ImVec4 LINE_COLOR = ImVec4(1, 1, 1, 1);
    static constexpr ImVec4 PERFECT_COLOR = ImVec4(1, 1, 1, 0.50);
    static constexpr auto LINE_HEIGHT = 5.0f;
    static constexpr auto LINE_WIDTH_BONUS = 10.0f;
    static constexpr auto TOAST_MESSAGE_SPEED = 1.0f;
    static constexpr auto ITEM_FALL_GRAVITY = 2400.0f;

    auto& dialogue = character.data.dialogue;
    auto& schema = character.data.skillCheckSchema;
    auto& itemSchema = character.data.itemSchema;
    auto& style = ImGui::GetStyle();
    auto drawList = ImGui::GetWindowDrawList();
    auto position = ImGui::GetCursorScreenPos();
    auto size = ImGui::GetContentRegionAvail();
    auto spacing = ImGui::GetTextLineHeightWithSpacing();
    auto& io = ImGui::GetIO();

    auto cursorPos = ImGui::GetCursorPos();

    ImGui::Text("Score: %i pts (%ix)", score, combo);
    auto bestString = std::format("Best: {} pts({}x)", highScore, bestCombo);
    ImGui::SetCursorPos(ImVec2(size.x - ImGui::CalcTextSize(bestString.c_str()).x, cursorPos.y));

    ImGui::Text("Best: %i pts (%ix)", highScore, bestCombo);

    if (score == 0 && isActive)
    {
      ImGui::SetCursorPos(ImVec2(style.WindowPadding.x, size.y - style.WindowPadding.y));
      ImGui::TextWrapped("Match the line to the colored areas with Space/click! Better performance, better rewards!");
    }

    auto barMin = ImVec2(position.x + (size.x * 0.5f) - (spacing * 0.5f), position.y + (spacing * 2.0f));
    auto barMax = ImVec2(barMin.x + (spacing * 2.0f), barMin.y + size.y - (spacing * 4.0f));
    auto endTimerProgress = (float)endTimer / endTimerMax;

    auto bgColor = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
    bgColor = imgui::to_imvec4(imgui::to_vec4(bgColor) * BG_COLOR_MULTIPLIER);
    drawList->AddRectFilled(barMin, barMax, ImGui::GetColorU32(bgColor));

    auto barWidth = barMax.x - barMin.x;
    auto barHeight = barMax.y - barMin.y;

    auto sub_ranges_get = [&](Range& range)
    {
      auto& min = range.min;
      auto& max = range.max;
      std::vector<Range> ranges{};

      auto baseHeight = max - min;
      auto center = (min + max) * 0.5f;

      int rangeCount{};

      for (auto& grade : schema.grades)
      {
        if (grade.isFailure) continue;

        auto scale = powf(0.5f, (float)rangeCount);
        auto halfHeight = baseHeight * scale * 0.5f;

        rangeCount++;

        ranges.push_back({center - halfHeight, center + halfHeight});
      }

      return ranges;
    };

    auto range_draw = [&](Range& range, float alpha = 1.0f)
    {
      auto subRanges = sub_ranges_get(range);

      for (int i = 0; i < (int)subRanges.size(); i++)
      {
        auto& subRange = subRanges[i];
        int layer = (int)subRanges.size() - 1 - i;

        ImVec2 rectMin = {barMin.x, barMin.y + subRange.min * barHeight};

        ImVec2 rectMax = {barMax.x, barMin.y + subRange.max * barHeight};

        ImVec4 color =
            i == (int)subRanges.size() - 1 ? PERFECT_COLOR : ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered);
        color.w = (color.w - (float)layer / subRanges.size()) * alpha;

        drawList->AddRectFilled(rectMin, rectMax, ImGui::GetColorU32(color));
      }
    };

    range_draw(challenge.range, isActive ? 1.0f : 0.0f);

    auto lineMin = ImVec2(barMin.x - LINE_WIDTH_BONUS, barMin.y + (barHeight * tryValue));
    auto lineMax = ImVec2(barMin.x + barWidth + LINE_WIDTH_BONUS, lineMin.y + LINE_HEIGHT);
    auto lineColor = LINE_COLOR;
    lineColor.w = isActive ? 1.0f : endTimerProgress;
    drawList->AddRectFilled(lineMin, lineMax, ImGui::GetColorU32(lineColor));

    if (!isActive && !isGameOver)
    {
      range_draw(queuedChallenge.range, 1.0f - endTimerProgress);

      auto queuedLineMin = ImVec2(barMin.x - LINE_WIDTH_BONUS, barMin.y + (barHeight * queuedChallenge.tryValue));
      auto queuedLineMax = ImVec2(barMin.x + barWidth + LINE_WIDTH_BONUS, queuedLineMin.y + LINE_HEIGHT);
      auto queuedLineColor = LINE_COLOR;
      queuedLineColor.w = 1.0f - endTimerProgress;
      drawList->AddRectFilled(queuedLineMin, queuedLineMax, ImGui::GetColorU32(queuedLineColor));
    }

    if (isActive)
    {
      tryValue += challenge.speed;

      if (tryValue > 1.0f || tryValue < 0.0f)
      {
        tryValue = tryValue > 1.0f ? 0.0f : tryValue < 0.0f ? 1.0f : tryValue;

        if (score > 0)
        {
          score--;
          schema.sounds.scoreLoss.play();
          auto toastMessagePosition =
              ImVec2(barMin.x - ImGui::CalcTextSize("-1").x - ImGui::GetTextLineHeightWithSpacing(), lineMin.y);
          toasts.emplace_back("-1", toastMessagePosition, schema.endTimerMax, schema.endTimerMax);
        }
      }

      ImGui::SetCursorScreenPos(barMin);
      auto barButtonSize = ImVec2(barMax.x - barMin.x, barMax.y - barMin.y);

      if (ImGui::IsKeyPressed(ImGuiKey_Space) ||
          WIDGET_FX(ImGui::InvisibleButton("##SkillCheckBar", barButtonSize, ImGuiButtonFlags_PressedOnClick)))
      {
        int gradeID{};

        auto subRanges = sub_ranges_get(challenge.range);

        for (int i = 0; i < (int)subRanges.size(); i++)
        {
          auto& subRange = subRanges[i];

          if (tryValue >= subRange.min && tryValue <= subRange.max)
            gradeID = std::min((int)gradeID + 1, (int)schema.grades.size() - 1);
        }

        gradeCounts[gradeID]++;
        totalPlays++;

        auto& grade = schema.grades.at(gradeID);
        grade.sound.play();

        if (text.is_interruptible() && grade.pool.is_valid()) text.set(dialogue.get(grade.pool), character);

        if (!grade.isFailure)
        {
          combo++;
          score += grade.value;

          if (score >= schema.rewardScore && !isRewardScoreAchieved)
          {
            schema.sounds.rewardScore.play();
            isRewardScoreAchieved = true;

            for (auto& itemID : itemSchema.skillCheckRewardItemPool)
            {
              inventory.values[itemID]++;
              if (!itemActors.contains(itemID))
              {
                itemActors[itemID] = Actor(itemSchema.anm2s[itemID], {}, Actor::SET);
                itemRects[itemID] = itemActors[itemID].rect();
              }
              auto rect = itemRects[itemID];
              auto rectSize = vec2(rect.z, rect.w);
              auto previewScale = (rectSize.x <= 0.0f || rectSize.y <= 0.0f || size.x <= 0.0f || size.y <= 0.0f ||
                                   !std::isfinite(rectSize.x) || !std::isfinite(rectSize.y))
                                      ? 0.0f
                                      : std::min(size.x / rectSize.x, size.y / rectSize.y);
              previewScale = std::min(1.0f, previewScale);
              auto previewSize = rectSize * previewScale;
              auto minX = position.x;
              auto maxX = position.x + size.x - previewSize.x;
              auto spawnX = minX >= maxX ? position.x : math::random_in_range(minX, maxX);
              auto spawnY = position.y - previewSize.y - math::random_in_range(0.0f, size.y);
              items.push_back({itemID, ImVec2(spawnX, spawnY), 0.0f});
            }

            auto toastMessagePosition =
                ImVec2(barMin.x - ImGui::CalcTextSize("Fantastic score!\nCongratulations!").x -
                           ImGui::GetTextLineHeightWithSpacing(),
                       lineMin.y + (ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y));
            toasts.emplace_back("Fantastic score! Congratulations!", toastMessagePosition, schema.endTimerMax,
                                schema.endTimerMax);
          }

          if (score > highScore)
          {
            highScore = score;

            if (isHighScoreAchieved && !isHighScoreAchievedThisRun)
            {
              isHighScoreAchievedThisRun = true;
              schema.sounds.highScore.play();
              auto toastMessagePosition =
                  ImVec2(barMin.x - ImGui::CalcTextSize("High Score!").x - ImGui::GetTextLineHeightWithSpacing(),
                         lineMin.y + ImGui::GetTextLineHeightWithSpacing());
              toasts.emplace_back("High Score!", toastMessagePosition, schema.endTimerMax, schema.endTimerMax);
            }
          }

          if (combo > bestCombo) bestCombo = combo;

          auto rewardBonus = (schema.rewardGradeBonus * score) + (schema.rewardGradeBonus * grade.value);
          while (rewardBonus > 0.0f)
          {
            const resource::xml::Item::Pool* pool{};
            int rewardID{-1};
            int rarityID{-1};
            auto chanceBonus = std::max(1.0f, (float)grade.value);

            for (auto& id : itemSchema.rarityIDsSortedByChance)
            {
              auto& rarity = itemSchema.rarities[id];
              if (rarity.chance <= 0.0f) continue;

              if (math::random_percent_roll(rarity.chance * chanceBonus))
              {
                pool = &itemSchema.pools[id];
                rarityID = id;
                break;
              }
            }

            if (pool && !pool->empty())
            {
              rewardID = (*pool)[(int)math::random_roll((float)pool->size())];
              auto& rarity = itemSchema.rarities.at(rarityID);

              rarity.sound.play();
              inventory.values[rewardID]++;
              if (!itemActors.contains(rewardID))
              {
                itemActors[rewardID] = Actor(itemSchema.anm2s[rewardID], {}, Actor::SET);
                itemRects[rewardID] = itemActors[rewardID].rect();
              }
              auto rect = itemRects[rewardID];
              auto rectSize = vec2(rect.z, rect.w);
              auto previewScale = (rectSize.x <= 0.0f || rectSize.y <= 0.0f || size.x <= 0.0f || size.y <= 0.0f ||
                                   !std::isfinite(rectSize.x) || !std::isfinite(rectSize.y))
                                      ? 0.0f
                                      : std::min(size.x / rectSize.x, size.y / rectSize.y);
              previewScale = std::min(1.0f, previewScale);
              auto previewSize = rectSize * previewScale;
              auto minX = position.x;
              auto maxX = position.x + size.x - previewSize.x;
              auto spawnX = minX >= maxX ? position.x : math::random_in_range(minX, maxX);
              auto spawnY = position.y - previewSize.y - math::random_in_range(0.0f, size.y);
              items.push_back({rewardID, ImVec2(spawnX, spawnY), 0.0f});
            }

            rewardBonus -= 1.0f;
          }
        }
        else
        {
          score = 0;
          combo = 0;
          if (isHighScoreAchieved) schema.sounds.highScoreLoss.play();
          if (highScore > 0) isHighScoreAchieved = true;
          isRewardScoreAchieved = false;
          isHighScoreAchievedThisRun = true;
          highScoreStart = highScore;
          isGameOver = true;
        }

        endTimerMax = grade.isFailure ? schema.endTimerFailureMax : schema.endTimerMax;
        isActive = false;
        endTimer = endTimerMax;

        queuedChallenge = challenge_generate(character);

        auto string = grade.isFailure ? grade.name : std::format("{} (+{})", grade.name, grade.value);
        auto toastMessagePosition =
            ImVec2(barMin.x - ImGui::CalcTextSize(string.c_str()).x - ImGui::GetTextLineHeightWithSpacing(), lineMin.y);
        toasts.emplace_back(string, toastMessagePosition, endTimerMax, endTimerMax);
      }
    }
    else
    {
      endTimer--;
      if (endTimer <= 0)
      {
        challenge = queuedChallenge;
        tryValue = challenge.tryValue;
        isActive = true;
        isGameOver = false;
      }
    }

    for (int i = 0; i < (int)toasts.size(); i++)
    {
      auto& toastMessage = toasts[i];

      toastMessage.position.y -= TOAST_MESSAGE_SPEED;

      auto textColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
      textColor.w = ((float)toastMessage.time / toastMessage.timeMax);

      drawList->AddText(toastMessage.position, ImGui::GetColorU32(textColor), toastMessage.message.c_str());

      toastMessage.time--;

      if (toastMessage.time <= 0) toasts.erase(toasts.begin() + i--);
    }

    auto gravity = ITEM_FALL_GRAVITY;
    auto windowMin = position;
    auto windowMax = ImVec2(position.x + size.x, position.y + size.y);
    ImGui::PushClipRect(windowMin, windowMax, true);
    for (int i = 0; i < (int)items.size(); i++)
    {
      auto& fallingItem = items[i];
      if (!itemActors.contains(fallingItem.id))
      {
        items.erase(items.begin() + i--);
        continue;
      }

      auto rect = itemRects[fallingItem.id];
      auto rectSize = vec2(rect.z, rect.w);
      auto previewScale = (rectSize.x <= 0.0f || rectSize.y <= 0.0f || size.x <= 0.0f || size.y <= 0.0f ||
                           !std::isfinite(rectSize.x) || !std::isfinite(rectSize.y))
                              ? 0.0f
                              : std::min(size.x / rectSize.x, size.y / rectSize.y);
      previewScale = std::min(1.0f, previewScale);
      auto previewSize = rectSize * previewScale;
      auto canvasSize = ivec2(std::max(1.0f, previewSize.x), std::max(1.0f, previewSize.y));

      if (!itemCanvases.contains(fallingItem.id))
        itemCanvases.emplace(fallingItem.id, Canvas(canvasSize, Canvas::FLIP));
      auto& canvas = itemCanvases[fallingItem.id];
      canvas.zoom = math::to_percent(previewScale);
      canvas.pan = vec2(rect.x, rect.y);
      canvas.bind();
      canvas.size_set(canvasSize);
      canvas.clear();

      itemActors[fallingItem.id].render(resources.shaders[shader::TEXTURE], resources.shaders[shader::RECT], canvas);
      canvas.unbind();

      auto min = fallingItem.position;
      auto max = ImVec2(fallingItem.position.x + previewSize.x, fallingItem.position.y + previewSize.y);
      drawList->AddImage(canvas.texture, min, max);

      fallingItem.velocity += gravity * io.DeltaTime;
      fallingItem.position.y += fallingItem.velocity * io.DeltaTime;
      if (fallingItem.position.y > position.y + size.y) items.erase(items.begin() + i--);
    }
    ImGui::PopClipRect();
  }
}
