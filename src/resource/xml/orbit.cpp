#include "orbit.hpp"

#include "../../log.hpp"
#include "util.hpp"

#include <format>

using namespace tinyxml2;
using namespace game::util;

namespace game::resource::xml
{
  Orbit::Orbit(const util::physfs::Path& path, Dialogue& dialogue)
  {
    XMLDocument document;

    if (document_load(path, document) != XML_SUCCESS)
    {
      logger.error(std::format("Unable to initialize orbit schema: {} ({})", path.c_str(), document.ErrorStr()));
      return;
    }

    auto archive = path.directory_get();

    if (auto root = document.RootElement())
    {
      std::string textureRootPath{};
      query_string_attribute(root, "TextureRootPath", &textureRootPath);

      std::string soundRootPath{};
      query_string_attribute(root, "SoundRootPath", &soundRootPath);

      if (std::string_view(root->Name()) != "Orbit")
      {
        logger.error(std::format("Orbit schema root element is not Orbit: {}", path.c_str()));
        return;
      }

      root->QueryIntAttribute("StartTime", &startTime);
      root->QueryFloatAttribute("RewardChanceBase", &rewardChanceBase);
      root->QueryFloatAttribute("RewardChanceScoreBonus", &rewardChanceScoreBonus);
      root->QueryFloatAttribute("RewardRollChanceBase", &rewardRollChanceBase);
      root->QueryFloatAttribute("RewardRollScoreBonus", &rewardRollScoreBonus);
      dialogue.query_pool_id(root, "HurtDialoguePoolID", poolHurt.id);
      dialogue.query_pool_id(root, "DeathDialoguePoolID", poolDeath.id);

      if (auto element = root->FirstChildElement("Player"))
      {
        query_anm2(element, "Anm2", archive, textureRootPath, player.anm2);
        query_string_attribute(element, "HitboxNull", &player.hitboxNull);
        element->QueryFloatAttribute("FollowerRadius", &player.followerRadius);
        element->QueryFloatAttribute("TargetAcceleration", &player.targetAcceleration);
        element->QueryFloatAttribute("RotationSpeed", &player.rotationSpeed);
        element->QueryFloatAttribute("RotationSpeedMax", &player.rotationSpeedMax);
        element->QueryFloatAttribute("RotationSpeedFriction", &player.rotationSpeedFriction);
        element->QueryIntAttribute("TimeAfterHurt", &player.timeAfterHurt);

        if (auto animationsElement = element->FirstChildElement("Animations"))
        {
          query_string_attribute(animationsElement, "Idle", &player.animations.idle);
          query_string_attribute(animationsElement, "Spawn", &player.animations.spawn);
          query_string_attribute(animationsElement, "Death", &player.animations.death);
        }
      }

      if (auto element = root->FirstChildElement("Follower"))
      {
        query_anm2(element, "Anm2", archive, textureRootPath, follower.anm2);
        query_string_attribute(element, "HitboxNull", &follower.hitboxNull);
        element->QueryFloatAttribute("TargetAcceleration", &follower.targetAcceleration);
        query_string_attribute(element, "OverrideTintLayer", &follower.overrideTintLayer);

        if (auto animationsElement = element->FirstChildElement("Animations"))
        {
          query_string_attribute(animationsElement, "Idle", &follower.animations.idle);
          query_string_attribute(animationsElement, "Spawn", &follower.animations.spawn);
          query_string_attribute(animationsElement, "Death", &follower.animations.death);
        }
      }

      if (auto element = root->FirstChildElement("Enemy"))
      {
        query_anm2(element, "Anm2", archive, textureRootPath, enemy.anm2);
        query_string_attribute(element, "HitboxNull", &enemy.hitboxNull);
        element->QueryFloatAttribute("Speed", &enemy.speed);
        element->QueryFloatAttribute("SpeedScoreBonus", &enemy.speedScoreBonus);
        element->QueryFloatAttribute("SpeedGainBase", &enemy.speedGainBase);
        element->QueryFloatAttribute("SpeedGainScoreBonus", &enemy.speedGainScoreBonus);
        element->QueryFloatAttribute("SpawnChanceBase", &enemy.spawnChanceBase);
        element->QueryFloatAttribute("SpawnChanceScoreBonus", &enemy.spawnChanceScoreBonus);
        element->QueryFloatAttribute("SpawnPadding", &enemy.spawnPadding);
        query_string_attribute(element, "OverrideTintLayer", &enemy.overrideTintLayer);

        if (auto animationsElement = element->FirstChildElement("Animations"))
        {
          query_string_attribute(animationsElement, "Idle", &enemy.animations.idle);
          query_string_attribute(animationsElement, "Spawn", &enemy.animations.spawn);
          query_string_attribute(animationsElement, "Death", &enemy.animations.death);
        }
      }

      if (auto element = root->FirstChildElement("Warning"))
      {
        query_anm2(element, "Anm2", archive, textureRootPath, warning.anm2);
        query_string_attribute(element, "OverrideTintLayer", &warning.overrideTintLayer);
      }

      if (auto element = root->FirstChildElement("Colors"))
      {
        for (auto child = element->FirstChildElement("Color"); child; child = child->NextSiblingElement("Color"))
        {
          Color color{};
          query_vec3(child, "ColorR", "ColorG", "ColorB", color.value);
          child->QueryIntAttribute("ScoreThreshold", &color.scoreThreshold);
          dialogue.query_pool_id(child, "DialoguePoolID", color.pool.id);
          colors.emplace_back(std::move(color));
        }
      }

      if (auto element = root->FirstChildElement("Sounds"))
      {
        query_sound_entry_collection(element, "LevelUp", archive, soundRootPath, sounds.levelUp);
        query_sound_entry_collection(element, "Hurt", archive, soundRootPath, sounds.hurt);
        query_sound_entry_collection(element, "HighScore", archive, soundRootPath, sounds.highScore);
        query_sound_entry_collection(element, "HighScoreLoss", archive, soundRootPath, sounds.highScoreLoss);
      }
    }

    isValid = true;
    logger.info(std::format("Initialized orbit schema: {}", path.c_str()));
  }

  bool Orbit::is_valid() const { return isValid; };
}
