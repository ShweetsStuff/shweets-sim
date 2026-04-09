#include "character.hpp"

#include <tinyxml2/tinyxml2.h>

#include "../../log.hpp"
#include "../../util/preferences.hpp"
#include "util.hpp"

#include <format>

using namespace tinyxml2;
using namespace game::util;

namespace game::resource::xml
{
  Character::Character(const std::filesystem::path& path)
  {
    XMLDocument document;

    physfs::Archive archive(path, path.filename().string());

    if (!archive.is_valid())
    {
      logger.error(std::format("Failed to initialize character from PhysicsFS archive: {} ({})", path.string(),
                               physfs::error_get()));
      return;
    }

    physfs::Path characterPath(archive + "/" + "character.xml");

    if (document_load(characterPath, document) != XML_SUCCESS) return;

    if (auto root = document.RootElement())
    {
      std::string textureRootPath{};
      query_string_attribute(root, "TextureRootPath", &textureRootPath);

      std::string soundRootPath{};
      query_string_attribute(root, "SoundRootPath", &soundRootPath);

      query_anm2(root, "Anm2", archive, textureRootPath, anm2);
      query_string_attribute(root, "Name", &name);

      query_vec3(root, "ColorR", "ColorG", "ColorB", color);

      root->QueryDoubleAttribute("Weight", &weight);
      root->QueryDoubleAttribute("WeightMax", &weightMax);

      root->QueryDoubleAttribute("Capacity", &capacity);
      root->QueryDoubleAttribute("CapacityMin", &capacityMin);
      root->QueryDoubleAttribute("CapacityMax", &capacityMax);
      root->QueryDoubleAttribute("CapacityMaxMultiplier", &capacityMaxMultiplier);
      root->QueryDoubleAttribute("CapacityIfOverStuffedOnDigestBonus", &capacityIfOverStuffedOnDigestBonus);

      root->QueryDoubleAttribute("CaloriesToKilogram", &caloriesToKilogram);

      root->QueryDoubleAttribute("DigestionRate", &digestionRate);
      root->QueryDoubleAttribute("DigestionRateMin", &digestionRateMin);
      root->QueryDoubleAttribute("DigestionRateMax", &digestionRateMax);
      root->QueryIntAttribute("DigestionTimerMax", &digestionTimerMax);

      root->QueryDoubleAttribute("EatSpeed", &eatSpeed);
      root->QueryDoubleAttribute("EatSpeedMin", &eatSpeedMin);
      root->QueryDoubleAttribute("EatSpeedMax", &eatSpeedMax);

      root->QueryDoubleAttribute("BlinkChance", &blinkChance);
      root->QueryDoubleAttribute("GurgleChance", &gurgleChance);
      root->QueryDoubleAttribute("GurgleCapacityMultiplier", &gurgleCapacityMultiplier);

      root->QueryIntAttribute("TextBlipPeriodBase", &textBlipPeriodBase);

      auto dialoguePath = physfs::Path(archive + "/" + "dialogue.xml");

      if (!dialoguePath.is_valid())
        logger.warning(std::format("No character dialogue.xml file found: {}", path.string()));
      else
        dialogue = Dialogue(dialoguePath);

      dialogue.query_pool_id(root, "DialoguePoolID", pool.id);

      if (auto element = root->FirstChildElement("AlternateSpritesheet"))
      {
        query_texture(element, "Texture", archive, textureRootPath, alternateSpritesheet.texture);
        query_sound(element, "Sound", archive, soundRootPath, alternateSpritesheet.sound);
        element->QueryIntAttribute("ID", &alternateSpritesheet.id);
        element->QueryFloatAttribute("ChanceOnNewGame", &alternateSpritesheet.chanceOnNewGame);
      }

      if (auto element = root->FirstChildElement("Animations"))
      {
        query_animation_entry_collection(element, "FinishFood", animations.finishFood);
        query_animation_entry_collection(element, "PostDigest", animations.postDigest);

        if (auto child = element->FirstChildElement("Idle"))
          query_string_attribute(child, "Animation", &animations.idle);

        if (auto child = element->FirstChildElement("IdleFull"))
          query_string_attribute(child, "Animation", &animations.idleFull);

        if (auto child = element->FirstChildElement("StageUp"))
          query_string_attribute(child, "Animation", &animations.stageUp);
      }

      if (auto element = root->FirstChildElement("Sounds"))
      {
        query_sound_entry_collection(element, "Blip", archive, soundRootPath, sounds.blip);
        query_sound_entry_collection(element, "Digest", archive, soundRootPath, sounds.digest);
        query_sound_entry_collection(element, "Gurgle", archive, soundRootPath, sounds.gurgle);
      }

      if (auto element = root->FirstChildElement("Overrides"))
      {
        if (auto child = element->FirstChildElement("Talk"))
        {
          query_layer_id(child, "LayerSource", anm2, talkOverride.layerSource);
          query_layer_id(child, "LayerDestination", anm2, talkOverride.layerDestination);
        }

        if (auto child = element->FirstChildElement("Blink"))
        {
          query_layer_id(child, "LayerSource", anm2, blinkOverride.layerSource);
          query_layer_id(child, "LayerDestination", anm2, blinkOverride.layerDestination);
        }
      }

      if (auto element = root->FirstChildElement("Stages"))
      {
        for (auto child = element->FirstChildElement("Stage"); child; child = child->NextSiblingElement("Stage"))
        {
          Stage stage{};
          child->QueryFloatAttribute("Threshold", &stage.threshold);
          child->QueryIntAttribute("AreaID", &stage.areaID);
          dialogue.query_pool_id(child, "DialoguePoolID", stage.pool.id);
          query_string_attribute(child, "AnimationAppendID", &stage.animationAppendID);
          stages.emplace_back(std::move(stage));
        }
      }

      if (auto element = root->FirstChildElement("EatAreas"))
      {
        for (auto child = element->FirstChildElement("EatArea"); child; child = child->NextSiblingElement("EatArea"))
        {
          EatArea eatArea{};
          query_null_id(child, "Null", anm2, eatArea.nullID);
          query_event_id(child, "Event", anm2, eatArea.eventID);
          query_string_attribute(child, "Animation", &eatArea.animation);

          eatAreas.emplace_back(std::move(eatArea));
        }
      }

      if (auto element = root->FirstChildElement("ExpandAreas"))
      {
        for (auto child = element->FirstChildElement("ExpandArea"); child;
             child = child->NextSiblingElement("ExpandArea"))
        {
          ExpandArea expandArea{};

          query_layer_id(child, "Layer", anm2, expandArea.layerID);
          query_null_id(child, "Null", anm2, expandArea.nullID);
          child->QueryFloatAttribute("ScaleAdd", &expandArea.scaleAdd);

          expandAreas.emplace_back(std::move(expandArea));
        }
      }

      if (auto element = root->FirstChildElement("InteractAreas"))
      {
        auto interact_type_id_get = [&](const std::string& typeName)
        {
          for (int i = 0; i < (int)interactTypeNames.size(); i++)
            if (interactTypeNames[i] == typeName) return i;

          interactTypeNames.emplace_back(typeName);
          return (int)interactTypeNames.size() - 1;
        };

        for (auto child = element->FirstChildElement("InteractArea"); child;
             child = child->NextSiblingElement("InteractArea"))
        {
          InteractArea interactArea{};

          if (child->FindAttribute("Layer")) query_layer_id(child, "Layer", anm2, interactArea.layerID);

          query_null_id(child, "Null", anm2, interactArea.nullID);
          query_string_attribute(child, "AnimationCursorHover", &interactArea.animationCursorHover);
          query_string_attribute(child, "AnimationCursorActive", &interactArea.animationCursorActive);
          query_sound_entry_collection(child, "Sound", archive, soundRootPath, interactArea.sound, "Path");
          dialogue.query_pool_id(child, "DialoguePoolID", interactArea.pool.id);
          dialogue.query_pool_id(child, "DialoguePoolIDFull", interactArea.poolFull.id);
          query_bool_attribute(child, "IsHold", &interactArea.isHold);
          child->QueryFloatAttribute("DigestionBonusRub", &interactArea.digestionBonusRub);
          child->QueryFloatAttribute("DigestionBonusClick", &interactArea.digestionBonusClick);
          child->QueryFloatAttribute("Time", &interactArea.time);
          child->QueryFloatAttribute("ScaleEffectAmplitude", &interactArea.scaleEffectAmplitude);
          child->QueryFloatAttribute("ScaleEffectCycles", &interactArea.scaleEffectCycles);

          std::string typeString{};
          query_string_attribute(child, "Type", &typeString);
          if (!typeString.empty()) interactArea.typeID = interact_type_id_get(typeString);

          interactAreas.emplace_back(std::move(interactArea));
        }
      }
    }

    if (auto itemSchemaPath = physfs::Path(archive + "/" + "items.xml"); itemSchemaPath.is_valid())
      itemSchema = Item(itemSchemaPath);
    else
      logger.warning(std::format("No character items.xml file found: {}", path.string()));

    if (auto areaSchemaPath = physfs::Path(archive + "/" + "areas.xml"); areaSchemaPath.is_valid())
      areaSchema = Area(areaSchemaPath);
    else
      logger.warning(std::format("No character areas.xml file found: {}", path.string()));

    if (auto menuSchemaPath = physfs::Path(archive + "/" + "menu.xml"); menuSchemaPath.is_valid())
      menuSchema = Menu(menuSchemaPath);
    else
      logger.warning(std::format("No character menu.xml file found: {}", path.string()));

    if (auto cursorSchemaPath = physfs::Path(archive + "/" + "cursor.xml"); cursorSchemaPath.is_valid())
      cursorSchema = Cursor(cursorSchemaPath);
    else
      logger.warning(std::format("No character cursor.xml file found: {}", path.string()));

    if (auto skillCheckSchemaPath = physfs::Path(archive + "/" + "skill_check.xml"); skillCheckSchemaPath.is_valid())
      skillCheckSchema = SkillCheck(skillCheckSchemaPath, dialogue);
    else
      logger.warning(std::format("No character skill_check.xml file found: {}", path.string()));

    if (auto dungeonSchemaPath = physfs::Path(archive + "/" + "dungeon.xml"); dungeonSchemaPath.is_valid())
      dungeonSchema = Dungeon(dungeonSchemaPath);
    else
      logger.warning(std::format("No character dungeon.xml file found: {}", path.string()));

    if (auto orbitSchemaPath = physfs::Path(archive + "/" + "orbit.xml"); orbitSchemaPath.is_valid())
      orbitSchema = Orbit(orbitSchemaPath, dialogue);
    else
      logger.warning(std::format("No character orbit.xml file found: {}", path.string()));

    if (auto stringsPath = physfs::Path(archive + "/" + "strings.xml"); stringsPath.is_valid())
      strings = Strings(stringsPath);

    logger.info(std::format("Initialized character: {}", name));

    this->path = path;
    save = Save(save_path_get());
  }

  std::filesystem::path Character::save_path_get()
  {
    auto savePath = path.stem();
    savePath = preferences::path() / "saves" / savePath.replace_extension(".save");
    std::filesystem::create_directories(savePath.parent_path());
    return savePath;
  }
}
