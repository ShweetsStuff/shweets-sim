#include "skill_check.hpp"

#include <imgui_internal.h>

#include "../../../../util/imgui.hpp"
#include "../../../../util/imgui/widget.hpp"
#include "../../../../util/math.hpp"

#include <cmath>
#include <format>

using namespace game::util;
using namespace game::entity;
using namespace game::resource;
using namespace game::resource::xml;
using namespace glm;

namespace game::state::play::menu::arcade
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

    Zone newZone{};

    auto zoneSize = std::max(schema.zoneMin, schema.zoneBase - (schema.zoneScoreBonus * score));
    newZone.min = math::random_max(1.0f - zoneSize);
    newZone.max = newZone.min + zoneSize;

    newChallenge.zone = newZone;
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

  void SkillCheck::reset(entity::Character& character)
  {
    challenge = challenge_generate(character);
    queuedChallenge = {};
    tryValue = challenge.tryValue;
    score = 0;
    combo = 0;
    endTimer = 0;
    endTimerMax = 0;
    highScoreStart = 0;
    isActive = true;
    isRewardScoreAchieved = false;
    isHighScoreAchieved = highScore > 0;
    isHighScoreAchievedThisRun = false;
    isGameOver = false;
    itemEffectManager = {};
  }

  void SkillCheck::tick() { itemEffectManager.tick(); }

  bool SkillCheck::update(Resources& resources, entity::Character& character, Inventory& inventory, Text& text,
                          Toasts& toasts)
  {
    static constexpr auto BG_COLOR_MULTIPLIER = 0.5f;
    static constexpr ImVec4 LINE_COLOR = ImVec4(1, 1, 1, 1);
    static constexpr ImVec4 PERFECT_COLOR = ImVec4(1, 1, 1, 0.50);
    static constexpr auto BAR_SPACING_MULTIPLIER = 1.5f;
    static constexpr auto LINE_HEIGHT = 5.0f;
    static constexpr auto LINE_WIDTH_BONUS = 10.0f;
    auto& dialogue = character.data.dialogue;
    auto& schema = character.data.skillCheckSchema;
    auto& itemSchema = character.data.itemSchema;
    auto& strings = character.data.strings;
    auto& style = ImGui::GetStyle();
    auto drawList = ImGui::GetWindowDrawList();
    auto position = ImGui::GetCursorScreenPos();
    auto size = ImGui::GetContentRegionAvail();
    auto spacing = ImGui::GetTextLineHeightWithSpacing() * BAR_SPACING_MULTIPLIER;
    auto& io = ImGui::GetIO();
    auto menuButtonHeight = ImGui::GetFrameHeightWithSpacing();
    size.y = std::max(0.0f, size.y - menuButtonHeight);
    auto bounds = ImVec4(position.x, position.y, size.x, size.y);

    auto cursorPos = ImGui::GetCursorPos();

    ImGui::Text(strings.get(Strings::ArcadeScoreComboFormat).c_str(), score, combo);
    auto bestString =
        std::vformat(strings.get(Strings::ArcadeBestScoreComboFormat), std::make_format_args(highScore, bestCombo));
    ImGui::SetCursorPos(ImVec2(size.x - ImGui::CalcTextSize(bestString.c_str()).x, cursorPos.y));

    ImGui::Text(strings.get(Strings::ArcadeBestScoreComboFormat).c_str(), highScore, bestCombo);

    if (score == 0 && isActive)
    {
      ImGui::SetCursorPos(ImVec2(style.WindowPadding.x, size.y - style.WindowPadding.y));
      ImGui::TextWrapped("%s", strings.get(Strings::SkillCheckInstructions).c_str());
    }

    auto barMin = ImVec2(position.x + (size.x * 0.5f) - (spacing * 0.5f), position.y + (spacing * 2.0f));
    auto barMax = ImVec2(barMin.x + (spacing * 2.0f), barMin.y + size.y - (spacing * 4.0f));
    auto endTimerProgress = (float)endTimer / endTimerMax;

    auto bgColor = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
    bgColor = imgui::to_imvec4(imgui::to_vec4(bgColor) * BG_COLOR_MULTIPLIER);
    drawList->AddRectFilled(barMin, barMax, ImGui::GetColorU32(bgColor));

    auto barWidth = barMax.x - barMin.x;
    auto barHeight = barMax.y - barMin.y;

    auto sub_zones_get = [&](Zone& zone)
    {
      auto& min = zone.min;
      auto& max = zone.max;
      std::vector<Zone> zones{};

      auto baseHeight = max - min;
      auto center = (min + max) * 0.5f;

      int zoneCount{};

      for (auto& grade : schema.grades)
      {
        if (grade.isFailure) continue;

        auto scale = powf(0.5f, (float)zoneCount);
        auto halfHeight = baseHeight * scale * 0.5f;

        zoneCount++;

        zones.push_back({center - halfHeight, center + halfHeight});
      }

      return zones;
    };

    auto zone_draw = [&](Zone& zone, float alpha = 1.0f)
    {
      auto subZones = sub_zones_get(zone);

      for (int i = 0; i < (int)subZones.size(); i++)
      {
        auto& subZone = subZones[i];
        int layer = (int)subZones.size() - 1 - i;

        ImVec2 rectMin = {barMin.x, barMin.y + subZone.min * barHeight};

        ImVec2 rectMax = {barMax.x, barMin.y + subZone.max * barHeight};

        ImVec4 color =
            i == (int)subZones.size() - 1 ? PERFECT_COLOR : ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered);
        color.w = (color.w - (float)layer / subZones.size()) * alpha;

        drawList->AddRectFilled(rectMin, rectMax, ImGui::GetColorU32(color));
      }
    };

    zone_draw(challenge.zone, isActive ? 1.0f : 0.0f);

    auto lineMin = ImVec2(barMin.x - LINE_WIDTH_BONUS, barMin.y + (barHeight * tryValue));
    auto lineMax = ImVec2(barMin.x + barWidth + LINE_WIDTH_BONUS, lineMin.y + LINE_HEIGHT);
    auto lineColor = LINE_COLOR;
    lineColor.w = isActive ? 1.0f : endTimerProgress;
    drawList->AddRectFilled(lineMin, lineMax, ImGui::GetColorU32(lineColor));

    if (!isActive && !isGameOver)
    {
      zone_draw(queuedChallenge.zone, 1.0f - endTimerProgress);

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
              ImVec2(barMin.x - ImGui::CalcTextSize(strings.get(Strings::ArcadeScoreLoss).c_str()).x -
                         ImGui::GetTextLineHeightWithSpacing(),
                     lineMin.y);
          toasts.spawn(strings.get(Strings::ArcadeScoreLoss), toastMessagePosition, schema.endTimerMax);
        }
      }

      ImGui::SetCursorScreenPos(barMin);
      auto barButtonSize = ImVec2(barMax.x - barMin.x, barMax.y - barMin.y);

      if (ImGui::IsKeyPressed(ImGuiKey_Space) ||
          WIDGET_FX(ImGui::InvisibleButton("##SkillCheckBar", barButtonSize, ImGuiButtonFlags_PressedOnClick)))
      {
        int gradeID{};

        auto subZones = sub_zones_get(challenge.zone);

        for (int i = 0; i < (int)subZones.size(); i++)
        {
          auto& subZone = subZones[i];

          if (tryValue >= subZone.min && tryValue <= subZone.max)
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
              itemRewards.item_give(itemID, inventory, itemEffectManager, itemSchema, bounds);

            auto toastMessagePosition =
                ImVec2(barMin.x - ImGui::CalcTextSize(strings.get(Strings::ArcadeRewardToast).c_str()).x -
                           ImGui::GetTextLineHeightWithSpacing(),
                       lineMin.y + (ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y));
            toasts.spawn(strings.get(Strings::ArcadeRewardToast), toastMessagePosition, schema.endTimerMax);
          }

          if (score > highScore)
          {
            highScore = score;

            if (isHighScoreAchieved && !isHighScoreAchievedThisRun)
            {
              isHighScoreAchievedThisRun = true;
              schema.sounds.highScore.play();
              auto toastMessagePosition =
                  ImVec2(barMin.x - ImGui::CalcTextSize(strings.get(Strings::ArcadeHighScoreToast).c_str()).x -
                             ImGui::GetTextLineHeightWithSpacing(),
                         lineMin.y + ImGui::GetTextLineHeightWithSpacing());
              toasts.spawn(strings.get(Strings::ArcadeHighScoreToast), toastMessagePosition, schema.endTimerMax);
            }
          }

          if (combo > bestCombo) bestCombo = combo;

          auto rewardChance = schema.rewardChanceBase + (schema.rewardChanceScoreBonus * score);
          auto rewardRollCount = schema.rewardRollChanceBase + (schema.rewardRollScoreBonus * score) +
                                 (schema.rewardRollGradeBonus * grade.value);
          itemRewards.reward_random_items_try(inventory, itemEffectManager, itemSchema, bounds, rewardChance,
                                              rewardRollCount);
        }
        else
        {
          score = 0;
          combo = 0;
          if (isHighScoreAchievedThisRun) schema.sounds.highScoreLoss.play();
          if (highScore > 0) isHighScoreAchieved = true;
          isRewardScoreAchieved = false;
          isHighScoreAchievedThisRun = false;
          highScoreStart = highScore;
          isGameOver = true;
        }

        endTimerMax = grade.isFailure ? schema.endTimerFailureMax : schema.endTimerMax;
        isActive = false;
        endTimer = endTimerMax;

        queuedChallenge = challenge_generate(character);

        auto string = grade.isFailure ? grade.name
                                      : std::vformat(strings.get(Strings::SkillCheckGradeSuccessTemplate),
                                                     std::make_format_args(grade.name, grade.value));
        auto toastMessagePosition =
            ImVec2(barMin.x - ImGui::CalcTextSize(string.c_str()).x - ImGui::GetTextLineHeightWithSpacing(), lineMin.y);
        toasts.spawn(string, toastMessagePosition, endTimerMax);
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

    toasts.update(drawList);

    itemEffectManager.render(resources, itemSchema, bounds, io.DeltaTime);

    ImGui::SetCursorScreenPos(ImVec2(position.x, position.y + size.y + ImGui::GetStyle().ItemSpacing.y));
    auto isMenuPressed = WIDGET_FX(ImGui::Button(strings.get(Strings::ArcadeMenuBackButton).c_str()));
    if (ImGui::IsItemHovered())
      ImGui::SetItemTooltip("%s", strings.get(Strings::ArcadeMenuBackButtonTooltip).c_str());
    return isMenuPressed;
  }
}
