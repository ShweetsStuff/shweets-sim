#pragma once

#include "../item_effect_manager.hpp"
#include "../../item/reward.hpp"
#include "../toasts.hpp"

#include "../../../../entity/character.hpp"
#include "../../../../resources.hpp"

#include "../inventory.hpp"
#include "../../text.hpp"

#include <imgui.h>
#include <map>
#include <vector>

namespace game::state::play::menu::arcade
{
  class SkillCheck
  {

  public:
    struct Zone
    {
      float min{};
      float max{};
    };

    struct Challenge
    {
      Zone zone{};
      float speed{};
      float tryValue{};
      int level{};
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

    game::state::play::menu::ItemEffectManager itemEffectManager{};
    game::state::play::item::Reward itemRewards{};

    SkillCheck() = default;
    SkillCheck(entity::Character&);
    Challenge challenge_generate(entity::Character&);
    void reset(entity::Character&);
    void tick();
    bool update(Resources&, entity::Character&, Inventory&, Text&, Toasts&);
    float accuracy_score_get(entity::Character&);
  };
}
