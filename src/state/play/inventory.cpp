#include "inventory.hpp"

#include <cmath>
#include <format>
#include <ranges>
#include <tuple>

#include "../../util/color.hpp"
#include "../../util/imgui.hpp"
#include "../../util/imgui/widget.hpp"
#include "../../util/math.hpp"

using namespace game::util;
using namespace game::util::imgui;
using namespace game::entity;
using namespace game::resource;
using namespace glm;

namespace game::state::play
{
  void Inventory::tick()
  {
    for (auto& [i, actor] : actors)
      actor.tick();
  }

  void Inventory::update(Resources& resources, ItemManager& itemManager, entity::Character& character)
  {
    static constexpr auto INFO_CHILD_HEIGHT_MULTIPLIER = 1.0f / 3.0f;

    auto& schema = character.data.itemSchema;

    auto quantity_get = [&](int itemID) -> int&
    {
      auto& quantity = values[itemID];
      quantity = glm::clamp(0, quantity, schema.quantityMax);
      return quantity;
    };

    auto is_possible_to_upgrade_get = [&](const resource::xml::Item::Entry& item)
    {
      return item.upgradeID.has_value() && item.upgradeCount.has_value() &&
             schema.idToStringMap.contains(*item.upgradeID);
    };

    auto is_able_to_upgrade_get = [&](const resource::xml::Item::Entry& item, int quantity)
    { return is_possible_to_upgrade_get(item) && quantity >= *item.upgradeCount; };

    auto item_use = [&](int itemID)
    {
      auto& item = schema.items[itemID];
      auto& category = schema.categories[item.categoryID];
      auto& quantity = quantity_get(itemID);

      if (quantity <= 0) return;

      if (category.isEdible)
      {
        if (itemManager.items.size() + 1 >= ItemManager::LIMIT)
          character.data.itemSchema.sounds.dispose.play();
        else
        {
          character.data.itemSchema.sounds.summon.play();
          itemManager.queuedItemIDs.emplace_back(itemID);
          quantity--;
          if (quantity <= 0) selectedItemID = -1;
        }
      }
      else if (item.isToggleSpritesheet)
      {
        character.spritesheet_set(character.spritesheetType == Character::NORMAL ? Character::ALTERNATE
                                                                                 : Character::NORMAL);
        character.data.alternateSpritesheet.sound.play();
        quantity--;
      }
    };

    auto item_upgrade = [&](int itemID, bool isAll)
    {
      auto& item = schema.items[itemID];
      auto& quantity = quantity_get(itemID);

      if (!is_possible_to_upgrade_get(item))
      {
        schema.sounds.upgradeFail.play();
        return;
      }

      if (!is_able_to_upgrade_get(item, quantity))
      {
        schema.sounds.upgradeFail.play();
        return;
      }

      if (isAll)
      {
        while (quantity >= *item.upgradeCount)
        {
          values.at(*item.upgradeID)++;
          quantity -= *item.upgradeCount;
        }
      }
      else
      {
        values.at(*item.upgradeID)++;
        quantity -= *item.upgradeCount;
      }

      schema.sounds.upgrade.play();

      if (quantity < *item.upgradeCount && selectedItemID == itemID) selectedItemID = *item.upgradeID;
    };

    auto item_canvas_get = [&](int itemID, ImVec2 size)
    {
      if (!actors.contains(itemID))
      {
        actors[itemID] = Actor(schema.anm2s[itemID], {}, Actor::SET);
        rects[itemID] = actors[itemID].rect();
      }

      auto& rect = rects[itemID];
      auto rectSize = vec2(rect.z, rect.w);
      auto previewScale = (size.x <= 0.0f || size.y <= 0.0f || rectSize.x <= 0.0f || rectSize.y <= 0.0f ||
                           !std::isfinite(rectSize.x) || !std::isfinite(rectSize.y))
                              ? 0.0f
                              : std::min(size.x / rectSize.x, size.y / rectSize.y);

      auto previewSize = rectSize * previewScale;
      auto canvasSize = ivec2(std::max(1.0f, previewSize.x), std::max(1.0f, previewSize.y));
      if (!canvases.contains(itemID)) canvases.emplace(itemID, Canvas(canvasSize, Canvas::FLIP));

      auto& canvas = canvases[itemID];
      canvas.zoom = math::to_percent(previewScale);
      canvas.pan = vec2(rect.x, rect.y);
      canvas.bind();
      canvas.size_set(canvasSize);
      canvas.clear();
      actors[itemID].render(resources.shaders[shader::TEXTURE], resources.shaders[shader::RECT], canvas);
      canvas.unbind();

      return std::tuple<Canvas&, glm::vec4&>(canvas, rect);
    };

    if (!itemManager.returnItemIDs.empty())
    {
      for (auto& id : itemManager.returnItemIDs)
        values[id]++;
      itemManager.returnItemIDs.clear();
    }

    if (ImGui::BeginChild("##Inventory Child", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None,
                          ImGuiWindowFlags_NoScrollbar))
    {
      auto inventoryCount = count();
      auto available = ImGui::GetContentRegionAvail();
      auto isItemSelected = selectedItemID >= 0 && selectedItemID < (int)schema.items.size();
      auto isInfoVisible = isItemSelected || inventoryCount == 0;
      auto infoChildHeight =
          isInfoVisible ? available.y * INFO_CHILD_HEIGHT_MULTIPLIER + ImGui::GetStyle().ItemSpacing.y * 2.0f : 0.0f;
      auto inventoryChildHeight =
          isInfoVisible ? available.y - infoChildHeight - ImGui::GetStyle().ItemSpacing.y : available.y;
      auto childSize = ImVec2(available.x, inventoryChildHeight);
      auto infoChildSize = ImVec2(available.x, infoChildHeight);

      if (ImGui::BeginChild("##Inventory List Child", childSize))
      {
        auto cursorPos = ImGui::GetCursorPos();
        auto cursorStartX = ImGui::GetCursorPosX();
        bool isAnyInventoryItemHovered{};

        auto size = ImVec2(SIZE, SIZE);

        for (int i = 0; i < (int)schema.items.size(); i++)
        {
          auto& item = schema.items[i];
          auto& quantity = quantity_get(i);
          auto& rarity = schema.rarities[item.rarityID];

          if (rarity.isHidden && quantity <= 0) continue;

          ImGui::PushID(i);

          ImGui::SetCursorPos(cursorPos);
          auto cursorScreenPos = ImGui::GetCursorScreenPos();
          auto [canvas, rect] = item_canvas_get(i, size);
          auto isSelected = selectedItemID == i;

          if (isSelected)
          {
            auto selectedColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
            ImGui::PushStyleColor(ImGuiCol_Button, selectedColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, selectedColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, selectedColor);
          }

          auto isPressed =
              WIDGET_FX(ImGui::ImageButton("##Image Button", canvas.texture, size, ImVec2(), ImVec2(1, 1), ImVec4(),
                                           quantity <= 0 ? ImVec4(0, 0, 0, 0.5f) : ImVec4(1, 1, 1, 1)));
          if (isSelected) ImGui::PopStyleColor(3);
          isAnyInventoryItemHovered = isAnyInventoryItemHovered || ImGui::IsItemHovered();
          if (isPressed) selectedItemID = i;
          if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && quantity > 0) item_use(i);

          ImGui::PushFont(ImGui::GetFont(), Font::BIG);

          auto text = std::format("x{}", quantity);
          auto textPos = ImVec2(cursorScreenPos.x + size.x - ImGui::CalcTextSize(text.c_str()).x,
                                cursorScreenPos.y + size.y - ImGui::GetTextLineHeightWithSpacing());
          ImGui::GetWindowDrawList()->AddText(textPos, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Text)),
                                              text.c_str());
          ImGui::PopFont();

          auto increment = ImGui::GetItemRectSize().x + ImGui::GetStyle().ItemSpacing.x;
          cursorPos.x += increment;

          if (cursorPos.x + increment > ImGui::GetContentRegionAvail().x)
          {
            cursorPos.x = cursorStartX;
            cursorPos.y += increment;
          }

          ImGui::PopID();
        }

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !isAnyInventoryItemHovered)
          selectedItemID = -1;
      }
      ImGui::EndChild();

      isItemSelected = selectedItemID >= 0 && selectedItemID < (int)schema.items.size();
      auto selectedQuantity = isItemSelected ? quantity_get(selectedItemID) : 0;
      auto isSelectedItemKnown = isItemSelected && selectedQuantity > 0;

      if (isInfoVisible &&
          ImGui::BeginChild("##Info Child", infoChildSize, ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar))
      {
        ImGui::Separator();
        auto isButtonChildVisible = selectedQuantity > 0;
        ImGui::PushFont(resources.font.get(), Font::BIG);
        auto buttonRowHeight = ImGui::GetFrameHeight();
        auto buttonChildHeight =
            isButtonChildVisible ? buttonRowHeight * 2.0f + ImGui::GetStyle().ItemSpacing.y * 5.0f : 0.0f;
        auto buttonChildSize = ImVec2(ImGui::GetContentRegionAvail().x, buttonChildHeight);
        auto infoBodySize =
            ImVec2(ImGui::GetContentRegionAvail().x,
                   ImGui::GetContentRegionAvail().y - buttonChildSize.y -
                       (isButtonChildVisible ? ImGui::GetStyle().ItemSpacing.y : 0.0f));
        ImGui::PopFont();

        if (ImGui::BeginChild("##Info Content Child", infoBodySize))
        {
          if (!isItemSelected)
          {
            ImGui::PushFont(ImGui::GetFont(), Font::BIG);
            ImGui::TextWrapped("%s", "Check the \"Arcade\" tab to earn rewards!");
            ImGui::PopFont();
          }
          else
          {
            auto& item = schema.items[selectedItemID];
            auto& category = schema.categories[item.categoryID];
            auto& rarity = schema.rarities[item.rarityID];

            if (isSelectedItemKnown)
            {
              ImGui::PushFont(ImGui::GetFont(), Font::BIG);
              ImGui::TextWrapped("%s (x%i)", item.name.c_str(), selectedQuantity);
              ImGui::PopFont();

              ImGui::Separator();
              ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(imgui::to_imvec4(color::GRAY)));
              ImGui::TextWrapped("-- %s (%s) --", category.name.c_str(), rarity.name.c_str());
              if (item.flavorID.has_value())
                ImGui::TextWrapped("Flavor: %s", schema.flavors[*item.flavorID].name.c_str());
              if (item.calories.has_value()) ImGui::TextWrapped("%0.0f kcal", *item.calories);
              if (item.digestionBonus.has_value())
              {
                if (*item.digestionBonus > 0)
                  ImGui::TextWrapped("Digestion Rate Bonus: +%0.2f%% / sec", *item.digestionBonus * 60.0f);
                else if (*item.digestionBonus < 0)
                  ImGui::TextWrapped("Digestion Rate Penalty: %0.2f%% / sec", *item.digestionBonus * 60.0f);
              }
              if (item.eatSpeedBonus.has_value())
              {
                if (*item.eatSpeedBonus > 0)
                  ImGui::TextWrapped("Eat Speed Bonus: +%0.2f%% / sec", *item.eatSpeedBonus);
                else if (*item.eatSpeedBonus < 0)
                  ImGui::TextWrapped("Eat Speed Penalty: %0.2f%% / sec", *item.eatSpeedBonus);
              }
              if (is_possible_to_upgrade_get(item))
                ImGui::TextWrapped("Upgrade: %ix -> %s", *item.upgradeCount,
                                   schema.idToStringMap.at(*item.upgradeID).c_str());
              ImGui::PopStyleColor();

              ImGui::Separator();
              ImGui::TextWrapped("%s", item.description.c_str());
            }
            else
            {
              ImGui::PushFont(ImGui::GetFont(), Font::BIG);
              ImGui::TextWrapped("%s", "???");
              ImGui::PopFont();
            }
          }
        }
        ImGui::EndChild();

        if (isButtonChildVisible &&
            ImGui::BeginChild("##Info Actions Child", buttonChildSize, ImGuiChildFlags_None,
                              ImGuiWindowFlags_NoScrollbar))
        {
          auto canUseSelectedItem = true;
          auto canUpgradeSelectedItem = is_able_to_upgrade_get(schema.items[selectedItemID], selectedQuantity);
          auto rowTwoButtonSize = row_widget_size_get(2);

          ImGui::Separator();
          ImGui::Dummy(ImVec2(0, ImGui::GetStyle().ItemSpacing.y));

          ImGui::PushFont(ImGui::GetFont(), Font::BIG);

          ImGui::BeginDisabled(!canUseSelectedItem);
          if (WIDGET_FX(ImGui::Button("Spawn", {ImGui::GetContentRegionAvail().x, 0}))) item_use(selectedItemID);
          ImGui::EndDisabled();

          ImGui::BeginDisabled(!canUpgradeSelectedItem);
          if (WIDGET_FX(ImGui::Button("Upgrade", rowTwoButtonSize))) item_upgrade(selectedItemID, false);
          ImGui::SameLine();
          if (WIDGET_FX(ImGui::Button("Upgrade All", rowTwoButtonSize))) item_upgrade(selectedItemID, true);
          ImGui::EndDisabled();

          ImGui::PopFont();
        }
        if (isButtonChildVisible) ImGui::EndChild();
      }
      if (isInfoVisible) ImGui::EndChild();
    }
    ImGui::EndChild();
  }

  int Inventory::count()
  {
    int count{};
    for (auto& [type, quantity] : values)
      count += quantity;
    return count;
  }
}
