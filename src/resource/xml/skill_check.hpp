#pragma once

#include <string>

#include "util.hpp"

#include "dialogue.hpp"

namespace game::resource::xml
{
  class SkillCheck
  {
  public:
    struct Grade
    {
      std::string name{};
      std::string namePlural{};
      int value{};
      float weight{};
      bool isFailure{};
      Audio sound{};
      Dialogue::PoolReference pool{};
    };

    struct Sounds
    {
      SoundEntryCollection fall{};
      SoundEntryCollection highScore{};
      SoundEntryCollection highScoreLoss{};
      SoundEntryCollection rewardScore{};
      SoundEntryCollection scoreLoss{};
    };

    Sounds sounds{};
    std::vector<Grade> grades{};

    float rewardChanceBase{0.01f};
    float rewardChanceScoreBonus{0.01f};
    float rewardRollChanceBase{1.0f};
    float rewardRollScoreBonus{0.05f};
    float rewardRollGradeBonus{0.05f};
    float speedMin{0.005f};
    float speedMax{0.075f};
    float speedScoreBonus{0.000025f};
    float zoneBase{0.75f};
    float zoneMin{0.10f};
    float zoneScoreBonus{0.0005f};
    int endTimerMax{20};
    int endTimerFailureMax{60};
    int rewardScore{999};

    bool isValid{};

    SkillCheck() = default;
    SkillCheck(const util::physfs::Path&, Dialogue&);

    bool is_valid() const;
  };
}
