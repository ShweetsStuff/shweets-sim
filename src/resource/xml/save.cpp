#include "save.hpp"
#include "util.hpp"

#include <tinyxml2/tinyxml2.h>

#include "../../log.hpp"

#include <format>

#ifdef __EMSCRIPTEN__
  #include "../../util/web_filesystem.hpp"
#endif

using namespace game::util;

using namespace tinyxml2;

namespace game::resource::xml
{
  Save::Save(const std::filesystem::path& path)
  {
    XMLDocument document;
    auto pathString = path.string();

    auto result = document.LoadFile(pathString.c_str());

    if (result == XML_ERROR_FILE_NOT_FOUND || result == XML_ERROR_FILE_COULD_NOT_BE_OPENED) return;

    if (result != XML_SUCCESS)
    {
      logger.error(std::format("Could not initialize character save file: {} ({})", pathString, document.ErrorStr()));
      return;
    }

    if (auto root = document.RootElement())
    {
      query_bool_attribute(root, "IsPostgame", &isPostgame);
      query_bool_attribute(root, "IsAlternateSpritesheet", &isAlternateSpritesheet);

      if (auto element = root->FirstChildElement("Character"))
      {

        element->QueryFloatAttribute("Weight", &weight);
        element->QueryFloatAttribute("Calories", &calories);
        element->QueryFloatAttribute("Capacity", &capacity);
        element->QueryFloatAttribute("DigestionRate", &digestionRate);
        element->QueryFloatAttribute("EatSpeed", &eatSpeed);
        query_bool_attribute(element, "IsDigesting", &isDigesting);
        element->QueryFloatAttribute("DigestionProgress", &digestionProgress);
        element->QueryIntAttribute("DigestionTimer", &digestionTimer);
        element->QueryFloatAttribute("TotalCaloriesConsumed", &totalCaloriesConsumed);
        element->QueryIntAttribute("TotalFoodItemsEaten", &totalFoodItemsEaten);
      }

      auto skillCheckElement = root->FirstChildElement("SkillCheck");
      if (!skillCheckElement) skillCheckElement = root->FirstChildElement("Play");
      if (skillCheckElement)
      {
        skillCheckElement->QueryIntAttribute("TotalPlays", &skillCheck.totalPlays);
        skillCheckElement->QueryIntAttribute("HighScore", &skillCheck.highScore);
        skillCheckElement->QueryIntAttribute("BestCombo", &skillCheck.bestCombo);

        if (auto child = skillCheckElement->FirstChildElement("Grades"))
        {
          for (auto gradeChild = child->FirstChildElement("Grade"); gradeChild;
               gradeChild = gradeChild->NextSiblingElement("Grade"))
          {
            int id{};
            gradeChild->QueryIntAttribute("ID", &id);
            gradeChild->QueryIntAttribute("Count", &skillCheck.gradeCounts[id]);
          }
        }
      }

      if (auto orbitElement = root->FirstChildElement("Orbit"))
      {
        orbitElement->QueryIntAttribute("HighScore", &orbit.highScore);
      }

      if (auto inventoryElement = root->FirstChildElement("Inventory"))
      {
        for (auto child = inventoryElement->FirstChildElement("Item"); child; child = child->NextSiblingElement("Item"))
        {
          int id{};
          int quantity{};
          child->QueryIntAttribute("ID", &id);
          child->QueryIntAttribute("Quantity", &quantity);

          inventory[id] = quantity;
        }
      }

      if (auto itemsElement = root->FirstChildElement("Items"))
      {
        for (auto child = itemsElement->FirstChildElement("Item"); child; child = child->NextSiblingElement("Item"))
        {
          Item item{};
          child->QueryIntAttribute("ID", &item.id);
          child->QueryIntAttribute("Durability", &item.durability);
          child->QueryFloatAttribute("PositionX", &item.position.x);
          child->QueryFloatAttribute("PositionY", &item.position.y);
          child->QueryFloatAttribute("VelocityX", &item.velocity.x);
          child->QueryFloatAttribute("VelocityY", &item.velocity.y);
          child->QueryFloatAttribute("Rotation", &item.rotation);
          items.emplace_back(std::move(item));
        }
      }
    }

    logger.info(std::format("Initialized character save file: {}", pathString));

    isValid = true;
  }

  bool Save::is_valid() const { return isValid; }

  void Save::serialize(const std::filesystem::path& path)
  {
    XMLDocument document;
    auto pathString = path.string();

    auto element = document.NewElement("Save");
    element->SetAttribute("IsPostgame", isPostgame ? "true" : "false");
    element->SetAttribute("IsAlternateSpritesheet", isAlternateSpritesheet ? "true" : "false");

    auto characterElement = element->InsertNewChildElement("Character");
    characterElement->SetAttribute("Weight", weight);
    characterElement->SetAttribute("Calories", calories);
    characterElement->SetAttribute("Capacity", capacity);
    characterElement->SetAttribute("DigestionRate", digestionRate);
    characterElement->SetAttribute("EatSpeed", eatSpeed);
    characterElement->SetAttribute("IsDigesting", isDigesting ? "true" : "false");
    characterElement->SetAttribute("DigestionProgress", digestionProgress);
    characterElement->SetAttribute("DigestionTimer", digestionTimer);
    characterElement->SetAttribute("TotalCaloriesConsumed", totalCaloriesConsumed);
    characterElement->SetAttribute("TotalFoodItemsEaten", totalFoodItemsEaten);

    auto skillCheckElement = element->InsertNewChildElement("SkillCheck");

    skillCheckElement->SetAttribute("TotalPlays", skillCheck.totalPlays);
    skillCheckElement->SetAttribute("HighScore", skillCheck.highScore);
    skillCheckElement->SetAttribute("BestCombo", skillCheck.bestCombo);

    auto gradesElement = skillCheckElement->InsertNewChildElement("Grades");

    for (auto& [i, count] : skillCheck.gradeCounts)
    {
      auto gradeElement = gradesElement->InsertNewChildElement("Grade");
      gradeElement->SetAttribute("ID", i);
      gradeElement->SetAttribute("Count", count);
    }

    auto orbitElement = element->InsertNewChildElement("Orbit");
    orbitElement->SetAttribute("HighScore", orbit.highScore);

    auto inventoryElement = element->InsertNewChildElement("Inventory");

    for (auto& [id, quantity] : inventory)
    {
      auto itemElement = inventoryElement->InsertNewChildElement("Item");

      itemElement->SetAttribute("ID", id);
      itemElement->SetAttribute("Quantity", quantity);
    }

    auto itemsElement = element->InsertNewChildElement("Items");

    for (auto& item : items)
    {
      auto itemElement = itemsElement->InsertNewChildElement("Item");

      itemElement->SetAttribute("ID", item.id);
      itemElement->SetAttribute("Durability", item.durability);
      itemElement->SetAttribute("PositionX", item.position.x);
      itemElement->SetAttribute("PositionY", item.position.y);
      itemElement->SetAttribute("VelocityX", item.velocity.x);
      itemElement->SetAttribute("VelocityY", item.velocity.y);
      itemElement->SetAttribute("Rotation", item.rotation);
    }

    document.InsertFirstChild(element);

    if (document.SaveFile(pathString.c_str()) != XML_SUCCESS)
    {
      logger.error(std::format("Failed to save character save file: {} ({})", pathString, document.ErrorStr()));
      return;
    }

    logger.info(std::format("Saved character save file: {}", pathString));

#ifdef __EMSCRIPTEN__
    web_filesystem::flush_async();
#endif
  }
}
