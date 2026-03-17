#include "skill_check.hpp"

#include "../../log.hpp"
#include "util.hpp"

#include <format>

using namespace tinyxml2;
using namespace game::util;

namespace game::resource::xml
{
  SkillCheck::SkillCheck(const physfs::Path& path, Dialogue& dialogue)
  {
    XMLDocument document;

    if (document_load(path, document) != XML_SUCCESS) return;

    auto archive = path.directory_get();

    if (auto root = document.RootElement())
    {
      std::string soundRootPath{};
      query_string_attribute(root, "SoundRootPath", &soundRootPath);

      root->QueryIntAttribute("RewardScore", &rewardScore);
      root->QueryFloatAttribute("RewardScoreBonus", &rewardScoreBonus);
      root->QueryFloatAttribute("RewardGradeBonus", &rewardGradeBonus);
      root->QueryFloatAttribute("RangeBase", &rangeBase);
      root->QueryFloatAttribute("RangeMin", &rangeMin);
      root->QueryFloatAttribute("RangeScoreBonus", &rangeScoreBonus);
      root->QueryFloatAttribute("SpeedMin", &speedMin);
      root->QueryFloatAttribute("SpeedMax", &speedMax);
      root->QueryFloatAttribute("SpeedScoreBonus", &speedScoreBonus);
      root->QueryIntAttribute("EndTimerMax", &endTimerMax);
      root->QueryIntAttribute("EndTimerFailureMax", &endTimerFailureMax);

      if (auto element = root->FirstChildElement("Sounds"))
      {
        query_sound_entry_collection(element, "Fall", archive, soundRootPath, sounds.fall);
        query_sound_entry_collection(element, "ScoreLoss", archive, soundRootPath, sounds.scoreLoss);
        query_sound_entry_collection(element, "HighScore", archive, soundRootPath, sounds.highScore);
        query_sound_entry_collection(element, "HighScoreLoss", archive, soundRootPath, sounds.highScoreLoss);
        query_sound_entry_collection(element, "RewardScore", archive, soundRootPath, sounds.rewardScore);
      }

      if (auto element = root->FirstChildElement("Grades"))
      {
        for (auto child = element->FirstChildElement("Grade"); child; child = child->NextSiblingElement("Grade"))
        {
          Grade grade{};
          query_string_attribute(child, "Name", &grade.name);
          query_string_attribute(child, "NamePlural", &grade.namePlural);
          child->QueryIntAttribute("Value", &grade.value);
          child->QueryFloatAttribute("Weight", &grade.weight);
          query_bool_attribute(child, "IsFailure", &grade.isFailure);
          query_sound(child, "Sound", archive, soundRootPath, grade.sound);
          dialogue.query_pool_id(child, "DialoguePoolID", grade.pool.id);
          grades.emplace_back(std::move(grade));
        }
      }
    }

    isValid = true;
    logger.info(std::format("Initialized skill check schema: {}", path.c_str()));
  }

  bool SkillCheck::is_valid() const { return isValid; };
}
