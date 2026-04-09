#pragma once

#include "../menu/inventory.hpp"
#include "../menu/item_effect_manager.hpp"

namespace game::state::play::item
{
  class Reward
  {
  public:
    static constexpr auto INVALID_ID = -1;

    int random_item_get(const resource::xml::Item& itemSchema, float chanceBonus = 1.0f);
    void item_give(int itemID, menu::Inventory& inventory, menu::ItemEffectManager& itemEffectManager,
                   const resource::xml::Item& itemSchema, const ImVec4& bounds,
                   menu::ItemEffectManager::Mode mode = menu::ItemEffectManager::FALL_DOWN);
    int reward_random_items_try(menu::Inventory& inventory, menu::ItemEffectManager& itemEffectManager,
                                const resource::xml::Item& itemSchema, const ImVec4& bounds, float rewardChance,
                                float rewardRollCount,
                                menu::ItemEffectManager::Mode mode = menu::ItemEffectManager::FALL_DOWN);
  };
}
