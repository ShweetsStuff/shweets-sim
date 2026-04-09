#include "reward.hpp"

#include "../../../util/math.hpp"

using namespace game::util;

namespace game::state::play::item
{
  int Reward::random_item_get(const resource::xml::Item& itemSchema, float chanceBonus)
  {
    const resource::xml::Item::Pool* pool{};

    for (auto& id : itemSchema.rarityIDsSortedByChance)
    {
      auto& rarity = itemSchema.rarities[id];
      if (rarity.chance <= 0.0f) continue;

      if (math::random_percent_roll(rarity.chance * chanceBonus))
      {
        pool = &itemSchema.pools.at(id);
        rarity.sound.play();
        break;
      }
    }

    if (!pool || pool->empty()) return INVALID_ID;
    return (*pool)[(int)math::random_roll((float)pool->size())];
  }

  void Reward::item_give(int itemID, menu::Inventory& inventory, menu::ItemEffectManager& itemEffectManager,
                         const resource::xml::Item& itemSchema, const ImVec4& bounds, menu::ItemEffectManager::Mode mode)
  {
    if (itemID < 0) return;

    inventory.values[itemID]++;
    itemEffectManager.spawn(itemID, itemSchema, bounds, mode);
  }

  int Reward::reward_random_items_try(menu::Inventory& inventory, menu::ItemEffectManager& itemEffectManager,
                                      const resource::xml::Item& itemSchema, const ImVec4& bounds, float rewardChance,
                                      float rewardRollCount, menu::ItemEffectManager::Mode mode)
  {
    if (!math::random_percent_roll(rewardChance)) return 0;

    auto rollCountWhole = std::max(0, (int)std::floor(rewardRollCount));
    auto rollCountFraction = std::max(0.0f, rewardRollCount - (float)rollCountWhole);
    auto rollCount = rollCountWhole + (math::random_percent_roll(rollCountFraction) ? 1 : 0);
    auto rewardedItemCount = 0;

    for (int i = 0; i < rollCount; i++)
    {
      auto itemID = random_item_get(itemSchema);
      if (itemID == INVALID_ID) continue;

      item_give(itemID, inventory, itemEffectManager, itemSchema, bounds, mode);
      rewardedItemCount++;
    }

    return rewardedItemCount;
  }
}
