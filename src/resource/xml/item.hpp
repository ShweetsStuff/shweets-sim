#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../audio.hpp"
#include "anm2.hpp"
#include "sound_entry.hpp"

namespace game::resource::xml
{
  class Item
  {
  public:
    static constexpr auto UNDEFINED = "???";

    struct Category
    {
      std::string name{};
      bool isEdible{};
    };

    struct Rarity
    {
      std::string name{UNDEFINED};
      float chance{};
      bool isHidden{};
      Audio sound{};
    };

    struct Flavor
    {
      std::string name{UNDEFINED};
    };

    struct Entry
    {
      std::string name{UNDEFINED};
      std::string description{UNDEFINED};
      int categoryID{};
      int rarityID{};
      std::optional<int> upgradeCount{};
      std::optional<int> upgradeID{};
      std::optional<int> flavorID;
      std::optional<float> calories{};
      std::optional<float> eatSpeedBonus{};
      std::optional<float> digestionBonus{};
      std::optional<float> gravity{};
      std::optional<int> chewCount{};
      bool isSkillCheckReward{};
      bool isToggleSpritesheet{};
    };

    struct Animations
    {
      std::string chew{};
    };

    struct Sounds
    {
      SoundEntryCollection bounce{};
      SoundEntryCollection return_{};
      SoundEntryCollection dispose{};
      SoundEntryCollection summon{};
      SoundEntryCollection upgrade{};
      SoundEntryCollection upgradeFail{};
    };

    std::unordered_map<std::string, int> categoryMap{};
    std::unordered_map<std::string, int> rarityMap{};
    std::unordered_map<std::string, int> flavorMap{};
    std::unordered_map<std::string, int> stringToIDMap{};
    std::unordered_map<int, std::string> idToStringMap{};

    using Pool = std::vector<int>;

    std::vector<Category> categories{};
    std::vector<Rarity> rarities{};
    std::vector<Flavor> flavors{};
    std::vector<Entry> items{};
    std::vector<Anm2> anm2s{};

    std::vector<int> rarityIDsSortedByChance{};
    std::unordered_map<int, Pool> pools{};
    Pool skillCheckRewardItemPool{};

    Animations animations{};
    Sounds sounds{};
    Anm2 baseAnm2{};
    int chewCount{2};
    int quantityMax{99};

    bool isValid{};

    Item() = default;
    Item(const std::filesystem::path&);
    Item(const util::physfs::Path&);

    bool is_valid() const;
  };
}
