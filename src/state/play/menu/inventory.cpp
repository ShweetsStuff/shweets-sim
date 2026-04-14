#include "inventory.hpp"
#include "../style.hpp"

#include <cmath>
#include <format>
#include <ranges>
#include <string_view>
#include <tuple>

#include "../../../util/color.hpp"
#include "../../../util/imgui.hpp"
#include "../../../util/imgui/style.hpp"
#include "../../../util/imgui/widget.hpp"
#include "../../../util/math.hpp"

using namespace game::util;
using namespace game::util::imgui;
using namespace game::entity;
using namespace game::resource;
using namespace glm;

namespace game::state::play::menu
{
  using Strings = resource::xml::Strings;

  void Inventory::tick()
  {
    for (auto& [i, actor] : actors)
      actor.tick();
  }

  void Inventory::update(Resources& resources, ItemManager& itemManager, entity::Character& character)
  {
    static constexpr auto INFO_CHILD_HEIGHT_MAX_MULTIPLIER = 0.5f;
    bool isSelectedItemPressed{};
    int pressedItemQuantity{-1};

    auto& schema = character.data.itemSchema;
    auto& strings = character.data.strings;

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

    auto item_header_draw = [&](const resource::xml::Item::Entry& item, int quantity)
    {
      ImGui::PushFont(ImGui::GetFont(), Font::HEADER_2);
      ImGui::TextWrapped("%s (x%i)", item.name.c_str(), quantity);
      ImGui::PopFont();
    };

    auto item_summary_draw = [&](const resource::xml::Item::Entry& item)
    {
      auto& category = schema.categories[item.categoryID];
      auto& rarity = schema.rarities[item.rarityID];
      auto durability = item.durability.value_or(schema.durability);

      ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(imgui::to_imvec4(color::GRAY)));
      ImGui::TextWrapped("-- %s (%s) --", category.name.c_str(), rarity.name.c_str());
      if (item.flavorID.has_value())
        ImGui::TextWrapped(strings.get(Strings::InventoryFlavorFormat).c_str(),
                           schema.flavors[*item.flavorID].name.c_str());
      if (item.calories.has_value())
        ImGui::TextWrapped(strings.get(Strings::InventoryCaloriesFormat).c_str(), *item.calories);
      ImGui::TextWrapped(strings.get(Strings::InventoryDurabilityFormat).c_str(), durability);
      if (item.capacityBonus.has_value())
        ImGui::TextWrapped(strings.get(Strings::InventoryCapacityBonusFormat).c_str(), *item.capacityBonus);
      if (item.digestionBonus.has_value())
      {
        if (*item.digestionBonus > 0)
          ImGui::TextWrapped(strings.get(Strings::InventoryDigestionRateBonusFormat).c_str(),
                             *item.digestionBonus * 60.0f);
        else if (*item.digestionBonus < 0)
          ImGui::TextWrapped(strings.get(Strings::InventoryDigestionRatePenaltyFormat).c_str(),
                             *item.digestionBonus * 60.0f);
      }
      if (item.eatSpeedBonus.has_value())
      {
        if (*item.eatSpeedBonus > 0)
          ImGui::TextWrapped(strings.get(Strings::InventoryEatSpeedBonusFormat).c_str(), *item.eatSpeedBonus);
        else if (*item.eatSpeedBonus < 0)
          ImGui::TextWrapped(strings.get(Strings::InventoryEatSpeedPenaltyFormat).c_str(), *item.eatSpeedBonus);
      }
      if (is_possible_to_upgrade_get(item))
        ImGui::TextWrapped(strings.get(Strings::InventoryUpgradePreviewFormat).c_str(), *item.upgradeCount,
                           schema.idToStringMap.at(*item.upgradeID).c_str());
      ImGui::PopStyleColor();
    };

    auto info_section_separator_height_get = [&]()
    { return ImGui::GetStyle().ItemSpacing.y + 1.0f; };

    auto info_section_separator_draw = [&]()
    {
      auto separatorHeight = info_section_separator_height_get();
      auto cursorScreenPos = ImGui::GetCursorScreenPos();
      auto width = ImGui::GetContentRegionAvail().x;
      auto y = cursorScreenPos.y + separatorHeight * 0.5f;
      ImGui::GetWindowDrawList()->AddLine(ImVec2(cursorScreenPos.x, y), ImVec2(cursorScreenPos.x + width, y),
                                          ImGui::GetColorU32(ImGuiCol_Separator));
      ImGui::Dummy(ImVec2(0.0f, separatorHeight));
    };

    auto item_tooltip_draw = [&](const resource::xml::Item::Entry& item, int quantity)
    {
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 24.0f);
      item_header_draw(item, quantity);
      ImGui::Separator();
      item_summary_draw(item);
      ImGui::Separator();
      ImGui::TextWrapped("%s", item.description.c_str());
      ImGui::PopTextWrapPos();
    };

    auto item_unknown_draw = [&]()
    {
      ImGui::PushFont(ImGui::GetFont(), Font::HEADER_2);
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 24.0f);
      ImGui::TextWrapped("%s", strings.get(Strings::InventoryUnknown).c_str());
      ImGui::PopTextWrapPos();
      ImGui::PopFont();
    };

    auto wrapped_text_height_get = [](std::string_view text, float wrapWidth)
    {
      auto safeWrapWidth = std::max(1.0f, wrapWidth);
      return ImGui::CalcTextSize(text.data(), text.data() + text.size(), false, safeWrapWidth).y;
    };

    auto item_header_height_get = [&](const resource::xml::Item::Entry& item, int quantity, float width)
    {
      float height{};

      ImGui::PushFont(ImGui::GetFont(), Font::HEADER_2);
      height += wrapped_text_height_get(std::format("{} (x{})", item.name, quantity), width);
      ImGui::PopFont();

      return height;
    };

    auto item_summary_height_get = [&](const resource::xml::Item::Entry& item, float width)
    {
      auto& category = schema.categories[item.categoryID];
      auto& rarity = schema.rarities[item.rarityID];
      auto durability = item.durability.value_or(schema.durability);
      auto itemSpacing = ImGui::GetStyle().ItemSpacing.y;
      float height{};
      int lineCount{};

      auto add_line_height = [&](std::string_view text)
      {
        if (lineCount > 0) height += itemSpacing;
        height += wrapped_text_height_get(text, width);
        lineCount++;
      };

      add_line_height(std::format("-- {} ({}) --", category.name, rarity.name));
      if (item.flavorID.has_value())
        add_line_height(std::vformat(strings.get(Strings::InventoryFlavorFormat),
                                     std::make_format_args(schema.flavors[*item.flavorID].name)));
      if (item.calories.has_value())
        add_line_height(std::vformat(strings.get(Strings::InventoryCaloriesFormat), std::make_format_args(*item.calories)));
      add_line_height(std::vformat(strings.get(Strings::InventoryDurabilityFormat), std::make_format_args(durability)));
      if (item.capacityBonus.has_value())
        add_line_height(
            std::vformat(strings.get(Strings::InventoryCapacityBonusFormat), std::make_format_args(*item.capacityBonus)));
      if (item.digestionBonus.has_value())
      {
        if (*item.digestionBonus > 0)
        {
          auto digestionRateBonus = *item.digestionBonus * 60.0f;
          add_line_height(
              std::vformat(strings.get(Strings::InventoryDigestionRateBonusFormat), std::make_format_args(digestionRateBonus)));
        }
        else if (*item.digestionBonus < 0)
        {
          auto digestionRatePenalty = *item.digestionBonus * 60.0f;
          add_line_height(std::vformat(strings.get(Strings::InventoryDigestionRatePenaltyFormat),
                                       std::make_format_args(digestionRatePenalty)));
        }
      }
      if (item.eatSpeedBonus.has_value())
      {
        if (*item.eatSpeedBonus > 0)
          add_line_height(
              std::vformat(strings.get(Strings::InventoryEatSpeedBonusFormat), std::make_format_args(*item.eatSpeedBonus)));
        else if (*item.eatSpeedBonus < 0)
          add_line_height(std::vformat(strings.get(Strings::InventoryEatSpeedPenaltyFormat),
                                       std::make_format_args(*item.eatSpeedBonus)));
      }
      if (is_possible_to_upgrade_get(item))
        add_line_height(std::vformat(strings.get(Strings::InventoryUpgradePreviewFormat),
                                     std::make_format_args(*item.upgradeCount, schema.idToStringMap.at(*item.upgradeID))));
      return height;
    };

    auto item_details_height_get = [&](const resource::xml::Item::Entry& item, int quantity, float width)
    {
      auto separatorHeight = info_section_separator_height_get();
      auto detailBodyHeight =
          std::max(item_summary_height_get(item, width), wrapped_text_height_get(item.description, width));
      return item_header_height_get(item, quantity, width) + separatorHeight + detailBodyHeight * 2.0f +
             separatorHeight;
    };

    auto item_unknown_height_get = [&](float width)
    {
      ImGui::PushFont(ImGui::GetFont(), Font::HEADER_2);
      auto height = wrapped_text_height_get(strings.get(Strings::InventoryUnknown), width);
      ImGui::PopFont();
      return height;
    };

    auto info_child_height_get = [&](ImVec2 available, bool isItemSelected, int selectedQuantity)
    {
      auto isInfoVisible = isItemSelected || count() == 0;
      if (!isInfoVisible) return 0.0f;

      auto infoWidth = std::max(1.0f, available.x - ImGui::GetStyle().WindowPadding.x * 2.0f);
      auto infoPadding = ImGui::GetStyle().WindowPadding.y * 2.0f;
      auto separatorHeight = info_section_separator_height_get();
      auto measuredBodyHeight = 0.0f;
      auto isSelectedItemKnown = isItemSelected && selectedQuantity > 0;

      if (!isItemSelected)
      {
        ImGui::PushFont(ImGui::GetFont(), Font::HEADER_2);
        measuredBodyHeight = wrapped_text_height_get(strings.get(Strings::InventoryEmptyHint), infoWidth);
        ImGui::PopFont();
      }
      else
      {
        auto& item = schema.items[selectedItemID];
        measuredBodyHeight =
            isSelectedItemKnown ? item_details_height_get(item, selectedQuantity, infoWidth) : item_unknown_height_get(infoWidth);
      }

      auto buttonChildHeight = 0.0f;
      if (selectedQuantity > 0)
      {
        ImGui::PushFont(resources.font.get(), Font::HEADER_2);
        auto buttonRowHeight = ImGui::GetFrameHeight();
        buttonChildHeight = buttonRowHeight * 2.0f + ImGui::GetStyle().ItemSpacing.y * 5.0f;
        ImGui::PopFont();
      }

      auto desiredInfoChildHeight = infoPadding + separatorHeight + measuredBodyHeight + buttonChildHeight +
                                    (buttonChildHeight > 0.0f ? separatorHeight : 0.0f);
      auto maxInfoChildHeight = available.y * INFO_CHILD_HEIGHT_MAX_MULTIPLIER;
      return glm::clamp(desiredInfoChildHeight, 0.0f, maxInfoChildHeight);
    };

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
      auto infoChildHeight = info_child_height_get(available, isItemSelected, isItemSelected ? quantity_get(selectedItemID) : 0);

      auto inventoryChildHeight =
          isInfoVisible ? std::max(0.0f, available.y - infoChildHeight - ImGui::GetStyle().ItemSpacing.y) : available.y;
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
          auto hasItemColor = item.color.has_value();

          if (rarity.isHidden && quantity <= 0) continue;

          ImGui::PushID(i);

          ImGui::SetCursorPos(cursorPos);
          auto cursorScreenPos = ImGui::GetCursorScreenPos();
          auto [canvas, rect] = item_canvas_get(i, size);
          auto isSelected = selectedItemID == i;
          if (hasItemColor) imgui::style::color_set(*item.color);

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
          if (isPressed)
          {
            isSelectedItemPressed = selectedItemID != i;
            selectedItemID = i;
            pressedItemQuantity = quantity;
          }
          if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && quantity > 0) item_use(i);
          if (!isSelected && ImGui::BeginItemTooltip())
          {
            if (quantity > 0)
              item_tooltip_draw(item, quantity);
            else
              item_unknown_draw();
            ImGui::EndTooltip();
          }

          ImGui::PushFont(ImGui::GetFont(), Font::HEADER_2);

          auto text = std::format("x{}", quantity);
          auto textPos = ImVec2(cursorScreenPos.x + size.x - ImGui::CalcTextSize(text.c_str()).x,
                                cursorScreenPos.y + size.y - ImGui::GetTextLineHeightWithSpacing());
          ImGui::GetWindowDrawList()->AddText(textPos, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Text)),
                                              text.c_str());
          ImGui::PopFont();
          if (hasItemColor) style::color_set(resources, character);

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
      isInfoVisible = isItemSelected || inventoryCount == 0;
      auto selectedQuantity =
          isItemSelected ? (isSelectedItemPressed && pressedItemQuantity >= 0 ? pressedItemQuantity : quantity_get(selectedItemID))
                         : 0;
      infoChildHeight = info_child_height_get(available, isItemSelected, selectedQuantity);
      infoChildSize = ImVec2(available.x, infoChildHeight);
      auto isSelectedItemKnown = isItemSelected && selectedQuantity > 0;
      auto selectedItemHasColor = isItemSelected && schema.items[selectedItemID].color.has_value();

      if (isInfoVisible && ImGui::BeginChild("##Info Child", infoChildSize, ImGuiChildFlags_None,
                                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
      {
        if (selectedItemHasColor) imgui::style::color_set(*schema.items[selectedItemID].color);
        info_section_separator_draw();
        auto isButtonChildVisible = selectedQuantity > 0;
        ImGui::PushFont(resources.font.get(), Font::HEADER_2);
        auto buttonRowHeight = ImGui::GetFrameHeight();
        auto buttonChildHeight =
            isButtonChildVisible ? buttonRowHeight * 2.0f + ImGui::GetStyle().ItemSpacing.y * 5.0f : 0.0f;
        ImGui::PopFont();
        auto separatorHeight = info_section_separator_height_get();

        if (!isItemSelected)
        {
          ImGui::PushFont(ImGui::GetFont(), Font::HEADER_2);
          ImGui::TextWrapped("%s", strings.get(Strings::InventoryEmptyHint).c_str());
          ImGui::PopFont();
        }
        else if (isSelectedItemKnown)
        {
          auto& selectedItem = schema.items[selectedItemID];
          auto contentWidth = std::max(1.0f, ImGui::GetContentRegionAvail().x);
          auto statsHeight = item_summary_height_get(selectedItem, contentWidth);
          auto descriptionHeight = wrapped_text_height_get(selectedItem.description, contentWidth);
          auto sharedDetailHeight = std::max(statsHeight, descriptionHeight);
          auto headerHeight = item_header_height_get(selectedItem, selectedQuantity, contentWidth);
          auto desiredInfoContentHeight =
              headerHeight + separatorHeight + sharedDetailHeight * 2.0f + separatorHeight;
          auto availableInfoContentHeight =
              std::max(0.0f, ImGui::GetContentRegionAvail().y - (isButtonChildVisible ? separatorHeight + buttonChildHeight : 0.0f));
          auto infoContentHeight = std::min(desiredInfoContentHeight, availableInfoContentHeight);

          item_header_draw(selectedItem, selectedQuantity);
          info_section_separator_draw();

          auto detailChildHeight = sharedDetailHeight;
          if (desiredInfoContentHeight > availableInfoContentHeight)
            detailChildHeight = std::max(0.0f, (infoContentHeight - headerHeight - separatorHeight - separatorHeight) * 0.5f);
          auto detailChildSize = ImVec2(ImGui::GetContentRegionAvail().x, detailChildHeight);

          if (ImGui::BeginChild("##Info Stats Child", detailChildSize))
          {
            if (isSelectedItemPressed) ImGui::SetScrollY(0.0f);
            item_summary_draw(selectedItem);
          }
          ImGui::EndChild();

          info_section_separator_draw();

          if (ImGui::BeginChild("##Info Description Child", detailChildSize))
          {
            if (isSelectedItemPressed) ImGui::SetScrollY(0.0f);
            ImGui::TextWrapped("%s", selectedItem.description.c_str());
          }
          ImGui::EndChild();

          if (isButtonChildVisible)
          {
            info_section_separator_draw();

            if (ImGui::BeginChild("##Info Actions Child", ImVec2(ImGui::GetContentRegionAvail().x, buttonChildHeight),
                                  ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar))
            {
              auto canUseSelectedItem = true;
              auto canUpgradeSelectedItem = is_able_to_upgrade_get(selectedItem, selectedQuantity);
              auto rowTwoButtonSize = row_widget_size_get(2);

              auto upgrade_item_name_get = [&]() -> std::string
              {
                if (!selectedItem.upgradeID.has_value()) return {};
                return schema.items.at(*selectedItem.upgradeID).name;
              };

              auto upgrade_tooltip_get = [&](bool isAll)
              {
                if (!is_possible_to_upgrade_get(selectedItem)) return strings.get(Strings::InventoryUpgradeNoPath);

                auto upgradeItemName = upgrade_item_name_get();
                auto upgradeCount = *selectedItem.upgradeCount;

                if (!canUpgradeSelectedItem)
                  return std::vformat(strings.get(Strings::InventoryUpgradeNeedsTemplate),
                                      std::make_format_args(upgradeCount, upgradeItemName));

                if (!isAll)
                  return std::vformat(strings.get(Strings::InventoryUpgradeOneTemplate),
                                      std::make_format_args(upgradeCount, upgradeItemName));

                auto upgradedCount = selectedQuantity / upgradeCount;
                return std::vformat(strings.get(Strings::InventoryUpgradeAllTemplate),
                                    std::make_format_args(upgradeCount, upgradedCount, upgradeItemName));
              };

              ImGui::PushFont(ImGui::GetFont(), Font::HEADER_2);

              ImGui::BeginDisabled(!canUseSelectedItem);
              if (WIDGET_FX(ImGui::Button(strings.get(Strings::InventorySpawnButton).c_str(),
                                          {ImGui::GetContentRegionAvail().x, 0})))
                item_use(selectedItemID);
              ImGui::EndDisabled();

              ImGui::BeginDisabled(!canUpgradeSelectedItem);
              if (WIDGET_FX(ImGui::Button(strings.get(Strings::InventoryUpgradeButton).c_str(), rowTwoButtonSize)))
                item_upgrade(selectedItemID, false);
              if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
              {
                ImGui::PushFont(ImGui::GetFont(), Font::NORMAL);
                ImGui::SetItemTooltip("%s", upgrade_tooltip_get(false).c_str());
                ImGui::PopFont();
              }
              ImGui::SameLine();
              if (WIDGET_FX(ImGui::Button(strings.get(Strings::InventoryUpgradeAllButton).c_str(), rowTwoButtonSize)))
                item_upgrade(selectedItemID, true);
              if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
              {
                ImGui::PushFont(ImGui::GetFont(), Font::NORMAL);
                ImGui::SetItemTooltip("%s", upgrade_tooltip_get(true).c_str());
                ImGui::PopFont();
              }
              ImGui::EndDisabled();

              ImGui::PopFont();
            }
            ImGui::EndChild();
          }
        }
        else
          item_unknown_draw();
        if (selectedItemHasColor) style::color_set(resources, character);
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
