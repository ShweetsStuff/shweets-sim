#include "item.hpp"

#include <ranges>
#include <tinyxml2.h>

#include <algorithm>
#include <utility>

#include "../../log.hpp"

#include <format>

#include "util.hpp"

using namespace tinyxml2;
using namespace game::util;

namespace game::resource::xml
{
  Item::Item(const physfs::Path& path)
  {
    XMLDocument document;

    if (document_load(path, document) != XML_SUCCESS) return;

    auto archive = path.directory_get();

    if (auto root = document.RootElement())
    {
      std::string textureRootPath{};
      query_string_attribute(root, "TextureRootPath", &textureRootPath);

      std::string soundRootPath{};
      query_string_attribute(root, "SoundRootPath", &soundRootPath);

      query_anm2(root, "BaseAnm2", archive, textureRootPath, baseAnm2, Anm2::NO_SPRITESHEETS);

      if (auto element = root->FirstChildElement("Categories"))
      {
        for (auto child = element->FirstChildElement("Category"); child; child = child->NextSiblingElement("Category"))
        {
          Category category{};
          query_string_attribute(child, "Name", &category.name);
          query_bool_attribute(child, "IsEdible", &category.isEdible);
          categoryMap[category.name] = (int)categories.size();
          categories.push_back(category);
        }
      }

      if (auto element = root->FirstChildElement("Rarities"))
      {
        for (auto child = element->FirstChildElement("Rarity"); child; child = child->NextSiblingElement("Rarity"))
        {
          Rarity rarity{};

          query_string_attribute(child, "Name", &rarity.name);
          child->QueryFloatAttribute("Chance", &rarity.chance);
          query_bool_attribute(child, "IsHidden", &rarity.isHidden);

          query_sound(child, "Sound", archive, soundRootPath, rarity.sound);

          rarityMap[rarity.name] = (int)rarities.size();
          rarities.emplace_back(std::move(rarity));
        }
      }

      if (auto element = root->FirstChildElement("Flavors"))
      {
        for (auto child = element->FirstChildElement("Flavor"); child; child = child->NextSiblingElement("Flavor"))
        {
          Flavor flavor{};
          query_string_attribute(child, "Name", &flavor.name);
          flavorMap[flavor.name] = (int)flavors.size();
          flavors.push_back(flavor);
        }
      }

      if (auto element = root->FirstChildElement("Animations"))
      {
        if (auto child = element->FirstChildElement("Chew"))
          query_string_attribute(child, "Animation", &animations.chew);
      }

      if (auto element = root->FirstChildElement("Sounds"))
      {
        query_sound_entry_collection(element, "Bounce", archive, soundRootPath, sounds.bounce);
        query_sound_entry_collection(element, "Dispose", archive, soundRootPath, sounds.dispose);
        query_sound_entry_collection(element, "Return", archive, soundRootPath, sounds.return_);
        query_sound_entry_collection(element, "Summon", archive, soundRootPath, sounds.summon);
        query_sound_entry_collection(element, "Upgrade", archive, soundRootPath, sounds.upgrade);
        query_sound_entry_collection(element, "UpgradeFail", archive, soundRootPath, sounds.upgradeFail);
      }

      if (auto element = root->FirstChildElement("Items"))
      {
        int id{};

        for (auto child = element->FirstChildElement("Item"); child; child = child->NextSiblingElement("Item"))
        {
          std::string name{};
          query_string_attribute(child, "Name", &name);
          stringToIDMap[name] = id;
          idToStringMap[id] = name;
          id++;
        }
      }

      if (auto element = root->FirstChildElement("Items"))
      {
        std::string itemTextureRootPath{};
        query_string_attribute(element, "TextureRootPath", &itemTextureRootPath);

        element->QueryIntAttribute("Durability", &durability);
        element->QueryIntAttribute("QuantityMax", &quantityMax);

        for (auto child = element->FirstChildElement("Item"); child; child = child->NextSiblingElement("Item"))
        {
          Entry item{};

          query_string_attribute(child, "Name", &item.name);
          query_string_attribute(child, "Description", &item.description);

          query_float_optional_attribute(child, "Calories", item.calories);
          query_float_optional_attribute(child, "CapacityBonus", item.capacityBonus);
          query_optional_vec3(child, "ColorR", "ColorG", "ColorB", item.color);
          query_float_optional_attribute(child, "DigestionBonus", item.digestionBonus);
          query_float_optional_attribute(child, "EatSpeedBonus", item.eatSpeedBonus);
          query_float_optional_attribute(child, "Gravity", item.gravity);

          query_int_optional_attribute(child, "Durability", item.durability);

          if (child->FindAttribute("UpgradeID"))
          {
            std::string upgradeIDString{};
            query_string_attribute(child, "UpgradeID", &upgradeIDString);

            if (!upgradeIDString.empty() && stringToIDMap.contains(upgradeIDString))
              item.upgradeID = stringToIDMap[upgradeIDString];
            else if (upgradeIDString.empty())
              logger.warning(std::format("Empty UpgradeID ({})", item.name));
            else
              logger.warning(std::format("Could not find item ID for UpgradeID: {} ({})", upgradeIDString, item.name));

            query_int_optional_attribute(child, "UpgradeCount", item.upgradeCount);
          }

          query_bool_attribute(child, "IsSkillCheckReward", &item.isSkillCheckReward);
          query_bool_attribute(child, "IsToggleSpritesheet", &item.isToggleSpritesheet);

          std::string categoryString{};
          query_string_attribute(child, "Category", &categoryString);
          item.categoryID = categoryMap.contains(categoryString) ? categoryMap[categoryString] : -1;

          std::string rarityString{};
          query_string_attribute(child, "Rarity", &rarityString);
          item.rarityID = rarityMap.contains(rarityString) ? rarityMap[rarityString] : -1;

          std::string flavorString{};
          query_string_attribute(child, "Flavor", &flavorString);
          if (flavorMap.contains(flavorString)) item.flavorID = flavorMap[flavorString];

          Texture texture{};
          query_texture(child, "Texture", archive, itemTextureRootPath, texture);

          Anm2 anm2{baseAnm2};
          if (child->FindAttribute("Anm2")) query_anm2(child, "Anm2", archive, textureRootPath, anm2);
          anm2.spritesheets.at(0).texture = std::move(texture);
          anm2s.emplace_back(std::move(anm2));
          items.emplace_back(std::move(item));
        }
      }
    }

    for (int i = 0; i < (int)items.size(); i++)
    {
      auto& item = items[i];
      pools[item.rarityID].emplace_back(i);
      if (item.isSkillCheckReward) skillCheckRewardItemPool.emplace_back(i);
    }

    for (int i = 0; i < (int)rarities.size(); i++)
    {
      rarityIDsSortedByChance.emplace_back(i);
    }
    std::stable_sort(rarityIDsSortedByChance.begin(), rarityIDsSortedByChance.end(),
                     [&](int a, int b) { return rarities[a].chance > rarities[b].chance; });

    isValid = true;
    logger.info(std::format("Initialized item schema: {}", path.c_str()));
  }

  bool Item::is_valid() const { return isValid; };
}
