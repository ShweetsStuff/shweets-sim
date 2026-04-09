#pragma once

#include <filesystem>

#include <map>
#include <vector>

#include <glm/glm.hpp>

namespace game::resource::xml
{
  class Save
  {
  public:
    struct SkillCheck
    {
      int totalPlays{};
      int highScore{};
      int bestCombo{};
      std::map<int, int> gradeCounts{};
    };

    struct Orbit
    {
      int highScore{};
    };

    struct Item
    {
      int id{};
      int durability{};
      glm::vec2 position{};
      glm::vec2 velocity{};
      float rotation{};
    };

    float weight{};
    float calories{};
    float capacity{};
    float eatSpeed{};
    float digestionRate{};
    float digestionProgress{};
    int digestionTimer{};
    bool isDigesting{};

    bool isAlternateSpritesheet{};

    float totalCaloriesConsumed{};
    int totalFoodItemsEaten{};
    SkillCheck skillCheck{};
    Orbit orbit{};

    std::map<int, int> inventory;
    std::vector<Item> items;

    bool isPostgame{};

    bool isValid{};

    Save() = default;
    Save(const std::filesystem::path&);
    void serialize(const std::filesystem::path&);
    bool is_valid() const;
  };
}
