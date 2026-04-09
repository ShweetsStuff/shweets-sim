#pragma once

#include <filesystem>
#include <vector>

#include "../audio.hpp"
#include "animation_entry.hpp"
#include "anm2.hpp"
#include "area.hpp"
#include "cursor.hpp"
#include "dialogue.hpp"
#include "dungeon.hpp"
#include "item.hpp"
#include "menu.hpp"
#include "orbit.hpp"
#include "save.hpp"
#include "skill_check.hpp"
#include "strings.hpp"

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
      std::string animationAppendID{};
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
      std::string animationCursorActive{};
      std::string animationCursorHover{};
      SoundEntryCollection sound{};

      int nullID{-1};
      int layerID{-1};
      int typeID{-1};
      bool isHold{};
      Dialogue::PoolReference pool{-1};
      Dialogue::PoolReference poolFull{-1};

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
      SoundEntryCollection blip{};
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
    Dungeon dungeonSchema{};
    Orbit orbitSchema{};
    Strings strings{};

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
    double weight{50};
    double weightMax{1000};
    double capacity{2000.0};
    double capacityMin{2000.0};
    double capacityMax{99999.0};
    double capacityMaxMultiplier{1.5};
    double capacityIfOverStuffedOnDigestBonus{0.25};
    double caloriesToKilogram{1000.0};
    double digestionRate{0.05};
    double digestionRateMin{0.0};
    double digestionRateMax{0.25};
    int digestionTimerMax{60};
    int textBlipPeriodBase{3};
    double eatSpeed{1.0};
    double eatSpeedMin{1.0};
    double eatSpeedMax{3.0};
    double blinkChance{1.0};
    double gurgleChance{1.0};
    double gurgleCapacityMultiplier{1.0};
    Dialogue::PoolReference pool{-1};

    Character() = default;
    Character(const std::filesystem::path&);
    std::filesystem::path save_path_get();
  };
}
