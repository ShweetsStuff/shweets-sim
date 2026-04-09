#pragma once

#include "dialogue.hpp"
#include "util.hpp"

namespace game::resource::xml
{
  class Orbit
  {
  public:
    struct Sounds
    {
      SoundEntryCollection levelUp{};
      SoundEntryCollection hurt{};
      SoundEntryCollection highScore{};
      SoundEntryCollection highScoreLoss{};
    };

    struct Animations
    {
      std::string idle{};
      std::string spawn{};
      std::string death{};
    };

    struct Player
    {
      Anm2 anm2{};
      Animations animations{};
      std::string hitboxNull{"Hitbox"};
      float followerRadius{};
      float targetAcceleration{6.0f};
      float rotationSpeed{0.01f};
      float rotationSpeedMax{0.10f};
      float rotationSpeedFriction{0.85f};
      int timeAfterHurt{30};
    };

    struct Follower
    {
      Anm2 anm2{};
      Animations animations{};
      std::string hitboxNull{"Hitbox"};
      float targetAcceleration{4.0f};
      std::string overrideTintLayer{"Default"};
    };

    struct Enemy
    {
      Anm2 anm2{};
      Animations animations{};
      std::string hitboxNull{"Hitbox"};
      float speed{4.0f};
      float speedScoreBonus{0.001f};
      float speedGainBase{0.01f};
      float speedGainScoreBonus{0.001f};
      float spawnChanceBase{0.35f};
      float spawnChanceScoreBonus{0.05f};
      float spawnPadding{8.0f};
      std::string overrideTintLayer{"Default"};
    };

    struct Warning
    {
      Anm2 anm2{};
      std::string overrideTintLayer{"Default"};
    };

    struct Color
    {
      glm::vec3 value{};
      int scoreThreshold{};
      Dialogue::PoolReference pool{};
    };

    Player player{};
    Follower follower{};
    Enemy enemy{};
    Warning warning{};
    Sounds sounds{};

    Dialogue::PoolReference poolHurt{};
    Dialogue::PoolReference poolDeath{};

    float rewardChanceBase{0.001f};
    float rewardChanceScoreBonus{0.001f};
    float rewardRollChanceBase{1.0f};
    float rewardRollScoreBonus{};

    std::vector<Color> colors{};
    int startTime{30};

    bool isValid{};

    Orbit() = default;
    Orbit(const util::physfs::Path&, Dialogue&);

    bool is_valid() const;
  };
}
