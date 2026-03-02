#include "inventory.hpp"

#include <cmath>
#include <format>
#include <ranges>

#include "../../util/color.hpp"
#include "../../util/imgui.hpp"
#include "../../util/imgui/widget.hpp"
#include "../../util/math.hpp"

using namespace game::util;
using namespace game::util::imgui;
using namespace game::entity;
using namespace game::resource;
using namespace glm;

namespace game::state::main
{
  void Inventory::tick()
  {
    for (auto& [i, actor] : actors)
      actor.tick();
  }

  void Inventory::update(Resources& resources, ItemManager& itemManager, entity::Character& character)
  {
    auto& schema = character.data.itemSchema;

    if (!itemManager.returnItemIDs.empty())
    {
      for (auto& id : itemManager.returnItemIDs)
        values[id]++;
      itemManager.returnItemIDs.clear();
    }

    if (ImGui::BeginChild("##Inventory Child"))
    {
      auto cursorPos = ImGui::GetCursorPos();
      auto cursorStartX = ImGui::GetCursorPosX();

      auto size = ImVec2(SIZE, SIZE);

      for (int i = 0; i < (int)schema.items.size(); i++)
      {
        auto& item = schema.items[i];
        auto& quantity = values[i];
        auto& category = schema.categories[item.categoryID];
        auto& calories = item.calories;
        auto& digestionBonus = item.digestionBonus;
        auto& eatSpeedBonus = item.eatSpeedBonus;
        auto& rarity = schema.rarities[item.rarityID];

        quantity = glm::clamp(0, quantity, schema.quantityMax);
        if (rarity.isHidden && quantity <= 0) continue;

        ImGui::PushID(i);

        ImGui::SetCursorPos(cursorPos);
        auto cursorScreenPos = ImGui::GetCursorScreenPos();

        if (!actors.contains(i))
        {
          actors[i] = Actor(schema.anm2s[i], {}, Actor::SET);
          rects[i] = actors[i].rect();
        }
        auto& rect = rects[i];
        auto rectSize = vec2(rect.z, rect.w);

        auto previewScale = (size.x <= 0.0f || size.y <= 0.0f || rectSize.x <= 0.0f || rectSize.y <= 0.0f ||
                             !std::isfinite(rectSize.x) || !std::isfinite(rectSize.y))
                                ? 0.0f
                                : std::min(size.x / rectSize.x, size.y / rectSize.y);

        auto previewSize = rectSize * previewScale;
        auto canvasSize = ivec2(std::max(1.0f, previewSize.x), std::max(1.0f, previewSize.y));
        if (!canvases.contains(i)) canvases.emplace((int)i, Canvas(canvasSize, Canvas::FLIP));
        auto& canvas = canvases[i];
        bool isPossibleToUpgrade = item.upgradeID.has_value() && item.upgradeCount.has_value() &&
                                   schema.idToStringMap.contains(*item.upgradeID);
        bool isAbleToUpgrade = isPossibleToUpgrade && quantity >= *item.upgradeCount;
        canvas.zoom = math::to_percent(previewScale);
        canvas.pan = vec2(rect.x, rect.y);
        canvas.bind();
        canvas.size_set(canvasSize);
        canvas.clear();

        actors[i].render(resources.shaders[shader::TEXTURE], resources.shaders[shader::RECT], canvas);
        canvas.unbind();

        ImGui::BeginDisabled(quantity < 1);
        if (WIDGET_FX(ImGui::ImageButton("##Image Button", canvas.texture, size, ImVec2(), ImVec2(1, 1), ImVec4(),
                                         quantity <= 0 ? ImVec4(0, 0, 0, 1) : ImVec4(1, 1, 1, 1))) &&
            quantity > 0)
        {
          if (ImGui::IsKeyDown(ImGuiMod_Shift))
          {
            if (isAbleToUpgrade)
            {
              if (ImGui::IsKeyDown(ImGuiMod_Ctrl))
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
            }
            else
              schema.sounds.upgradeFail.play();
          }
          else if (category.isEdible)
          {
            if (itemManager.items.size() + 1 >= ItemManager::LIMIT)
              character.data.itemSchema.sounds.dispose.play();
            else
            {
              character.data.itemSchema.sounds.summon.play();
              itemManager.queuedItemIDs.emplace_back(i);
              quantity--;
            }
          }
          else if (item.isToggleSpritesheet)
          {
            character.spritesheet_set(character.spritesheetType == Character::NORMAL ? Character::ALTERNATE
                                                                                     : Character::NORMAL);
            character.data.alternateSpritesheet.sound.play();
            quantity--;
          }
        }

        ImGui::EndDisabled();

        if (ImGui::BeginItemTooltip())
        {
          if (quantity > 0)
          {
            ImGui::PushFont(ImGui::GetFont(), Font::BIG);

            ImGui::Text("%s (x%i)", item.name.c_str(), quantity);

            ImGui::Separator();

            ImGui::PopFont();

            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(imgui::to_imvec4(color::GRAY)));

            ImGui::Text("-- %s (%s) --", category.name.c_str(), rarity.name.c_str());
            if (item.flavorID.has_value()) ImGui::Text("Flavor: %s", schema.flavors[*item.flavorID].name.c_str());
            if (calories.has_value()) ImGui::Text("%0.0f kcal", *calories);
            if (digestionBonus.has_value())
            {
              if (*digestionBonus > 0)
                ImGui::Text("Digestion Rate Bonus: +%0.2f%% / sec", *digestionBonus * 60.0f);
              else if (digestionBonus < 0)
                ImGui::Text("Digestion Rate Penalty: %0.2f%% / sec", *digestionBonus * 60.0f);
            }
            if (eatSpeedBonus.has_value())
            {
              if (*eatSpeedBonus > 0)
                ImGui::Text("Eat Speed Bonus: +%0.2f%% / sec", *eatSpeedBonus);
              else if (eatSpeedBonus < 0)
                ImGui::Text("Eat Speed Penalty: %0.2f%% / sec", *eatSpeedBonus);
            }

            if (isPossibleToUpgrade)
            {
              ImGui::Text("Upgrade: %ix -> %s", *item.upgradeCount, schema.idToStringMap.at(*item.upgradeID).c_str());
              if (isAbleToUpgrade)
                ImGui::TextUnformatted("(Shift + Click -> Upgrade)\n(Shift + Ctrl + Click -> Upgrade All)");
            }

            ImGui::PopStyleColor();

            ImGui::Separator();
            ImGui::TextUnformatted(item.description.c_str());
          }
          else
          {
            ImGui::PushFont(ImGui::GetFont(), Font::BIG);
            ImGui::TextUnformatted("???");
            ImGui::PopFont();
          }

          ImGui::EndTooltip();
        }

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

      if (count() == 0) ImGui::Text("Check the \"Play\" tab to earn rewards!");
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
