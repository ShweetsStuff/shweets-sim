#pragma once

#include "../../../render/canvas.hpp"
#include "../../../entity/actor.hpp"
#include "../../../entity/character.hpp"
#include "../../../resources.hpp"

#include "../inventory.hpp"
#include "../text.hpp"

#include <imgui.h>
#include <map>
#include <unordered_map>
#include <vector>

namespace game::state::play
{
  class SkillCheck
  {

  public:
    struct Range
    {
      float min{};
      float max{};
    };

    struct Challenge
    {
      Range range{};
      float speed{};
      float tryValue{};
      int level{};
    };

    struct Toast
    {
      std::string message{};
      ImVec2 position;
      int time{};
      int timeMax{};
    };

    struct Item
    {
      int id{-1};
      ImVec2 position{};
      float velocity{};
    };

    Challenge challenge{};
    Challenge queuedChallenge{};
    float tryValue{};

    int score{};
    int combo{};

    int endTimer{};
    int endTimerMax{};

    int highScoreStart{};

    int bestCombo{};
    int highScore{};
    int totalPlays{};
    std::map<int, int> gradeCounts{};

    bool isActive{true};
    bool isRewardScoreAchieved{false};
    bool isHighScoreAchieved{false};
    bool isHighScoreAchievedThisRun{false};
    bool isGameOver{};

    std::vector<Toast> toasts{};
    std::vector<Item> items{};
    std::unordered_map<int, entity::Actor> itemActors{};
    std::unordered_map<int, glm::vec4> itemRects{};
    std::unordered_map<int, Canvas> itemCanvases{};

    SkillCheck() = default;
    SkillCheck(entity::Character&);
    Challenge challenge_generate(entity::Character&);
    void tick();
    void update(Resources&, entity::Character&, Inventory&, Text&);
    float accuracy_score_get(entity::Character&);
  };
}
