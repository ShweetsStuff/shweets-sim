#pragma once

#include <filesystem>
#include <vector>

#include "../audio.hpp"
#include "animation_entry.hpp"
#include "anm2.hpp"
#include "area.hpp"
#include "cursor.hpp"
#include "dialogue.hpp"
#include "item.hpp"
#include "menu.hpp"
#include "skill_check.hpp"
#include "save.hpp"

namespace game::resource::xml
{
  class Character
  {
  public:
    struct Stage
    {
      float threshold{};
      int areaID{};
      Dialogue::PoolReference pool{-1};
    };

    struct EatArea
    {
      int nullID{-1};
      int eventID{-1};
      std::string animation{};
    };

    struct ExpandArea
    {
      int layerID{-1};
      int nullID{-1};
      float scaleAdd{};
    };

    struct InteractArea
    {
      std::string animation{};
      std::string animationFull{};
      std::string animationCursorActive{};
      std::string animationCursorHover{};
      SoundEntryCollection sound{};

      int nullID{-1};
      int layerID{-1};
      int typeID{-1};
      bool isHold{};
      Dialogue::PoolReference pool{-1};

      float digestionBonusRub{};
      float digestionBonusClick{};
      float time{};
      float scaleEffectAmplitude{};
      float scaleEffectCycles{};
    };

    struct Animations
    {
      AnimationEntryCollection finishFood{};
      AnimationEntryCollection postDigest{};

      std::string idle{};
      std::string idleFull{};
      std::string stageUp{};
    };

    struct Sounds
    {
      SoundEntryCollection gurgle{};
      SoundEntryCollection digest{};
    };

    struct Override
    {
      int layerSource{};
      int layerDestination{};
    };

    struct AlternateSpritesheet
    {
      Texture texture{};
      Audio sound{};
      int id{-1};
      float chanceOnNewGame{0.001f};
    };

    Anm2 anm2{};
    Area areaSchema{};
    Dialogue dialogue{};
    Item itemSchema{};
    Menu menuSchema{};
    Cursor cursorSchema{};
    SkillCheck skillCheckSchema{};

    Save save{};

    Animations animations{};
    Override talkOverride{};
    Override blinkOverride{};

    Sounds sounds{};

    glm::vec3 color{0.120f, 0.515f, 0.115f};

    std::vector<Stage> stages{};
    std::vector<ExpandArea> expandAreas{};
    std::vector<EatArea> eatAreas{};
    std::vector<std::string> interactTypeNames{};
    std::vector<InteractArea> interactAreas{};

    AlternateSpritesheet alternateSpritesheet{};

    std::string name{};
    std::filesystem::path path{};
    float weight{50};
    float capacity{2000.0f};
    float capacityMin{2000.0f};
    float capacityMax{99999.0f};
    float capacityMaxMultiplier{1.5f};
    float capacityIfOverStuffedOnDigestBonus{0.25f};
    float caloriesToKilogram{1000.0f};
    float digestionRate{0.05f};
    float digestionRateMin{0.0f};
    float digestionRateMax{0.25f};
    int digestionTimerMax{60};
    float eatSpeed{1.0f};
    float eatSpeedMin{1.0f};
    float eatSpeedMax{3.0f};
    float blinkChance{1.0f};
    float gurgleChance{1.0f};
    float gurgleCapacityMultiplier{1.0f};
    Dialogue::PoolReference pool{-1};

    Character() = default;
    Character(const std::filesystem::path&);
    std::filesystem::path save_path_get();
  };
}
