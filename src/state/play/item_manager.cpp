#include "item_manager.hpp"

#include <cmath>
#include <string>
#include <utility>

#include "../../util/math.hpp"
#include "../../util/vector.hpp"

#include <imgui.h>

using namespace game::resource;
using namespace game::util;
using namespace glm;

namespace game::state::play
{
  void ItemManager::update(entity::Character& character, entity::Cursor& cursor, AreaManager& areaManager, Text& text,
                           const glm::vec4& bounds, Canvas& canvas)
  {
    static constexpr float ROTATION_MAX = 90.0f;
    static constexpr float ROTATION_RETURN_SPEED = 120.0f;
    static constexpr float ROTATION_GROUND_DAMPING = 0.85f;
    static constexpr float ROTATION_HOLD_DELTA_ANGULAR_VELOCITY_MULTIPLIER = 0.1f;
    static constexpr float THROW_THRESHOLD = 10.0f;

    auto& schema = character.data.itemSchema;
    auto& cursorSchema = character.data.cursorSchema;
    auto& area = character.data.areaSchema.areas.at(areaManager.get(character));
    auto& friction = area.friction;
    auto& airResistance = area.airResistance;
    auto& dialogue = character.data.dialogue;
    auto isOverCapacity = character.is_over_capacity();

    auto cursorPosition = canvas.screen_position_convert(cursor.position);
    auto cursorDelta = cursorPosition - cursorPositionPrevious;

    auto isImguiCaptureMouse = ImGui::GetIO().WantCaptureMouse;

    auto isMouseLeftClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    auto isMouseLeftDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    auto isMouseLeftReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
    auto isMouseRightClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Right);
    auto isMouseRightDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);

    auto& io = ImGui::GetIO();

    if (isJustItemHoveredStopped)
    {
      cursor.queue_default_animation();
      isJustItemHoveredStopped = false;
    }

    if (isJustItemHeldStopped || isJustItemThrown)
    {
      cursor.queue_default_animation();
      if (!isJustItemThrown && character.queuedPlay.empty()) character.queue_idle_animation();
      isJustItemHeldStopped = false;
      isJustItemThrown = false;
    }

    isItemHoveredPrevious = isItemHovered;
    isItemHovered = false;
    if (isItemHovered != isItemHoveredPrevious && !isItemHovered) isJustItemHoveredStopped = true;

    for (auto& id : queuedItemIDs)
    {
      auto spawnBounds = character.rect();
      auto position = glm::vec2(math::random_in_range(spawnBounds.x, spawnBounds.x + spawnBounds.z),
                                math::random_in_range(spawnBounds.y, spawnBounds.y + spawnBounds.w));

      auto& itemSchema = character.data.itemSchema;
      items.emplace_back(itemSchema.anm2s.at(id), position, id);
    }
    queuedItemIDs.clear();

    if (isMouseRightDown)
    {
      auto animation = cursorSchema.animations.return_.get();
      if (animation) cursor.queue_play({*animation});
    }

    if (auto heldItem = vector::find(items, heldItemIndex))
    {
      auto& item = schema.items[heldItem->schemaID];
      auto& rotationOverride = heldItem->overrides[heldItem->rotationOverrideID];
      auto& rotation = *rotationOverride.frame.rotation;

      auto delta = cursorPositionPrevious - cursorPosition;
      heldItem->position = cursorPosition;
      heldItem->velocity = vec2();

      heldItem->angularVelocity -= delta.x * ROTATION_HOLD_DELTA_ANGULAR_VELOCITY_MULTIPLIER;
      heldItem->angularVelocity *= friction;
      rotation *= friction;
      rotation = glm::clamp(-ROTATION_MAX, rotation, ROTATION_MAX);

      if (schema.categories[item.categoryID].isEdible)
      {
        auto& chewCountMax = item.chewCount.has_value() ? *item.chewCount : schema.chewCount;
        auto caloriesChew = item.calories.has_value() ? *item.calories / (chewCountMax + 1) : 0;
        auto isCanEat = character.calories + caloriesChew <= character.max_capacity();

        if (isJustItemHeld)
        {
          if (isCanEat)
            text.set(dialogue.get(isOverCapacity ? dialogue.feedFull : dialogue.feed), character);
          else if (caloriesChew > character.capacity)
            text.set(dialogue.get(dialogue.lowCapacity), character);
          else
            text.set(dialogue.get(dialogue.full), character);
          isJustItemHeld = false;
        }

        for (auto& eatArea : character.data.eatAreas)
        {
          heldItem = vector::find(items, heldItemIndex);
          if (!heldItem) break;

          auto rect = character.null_frame_rect(eatArea.nullID);

          if (isCanEat && math::is_point_in_rectf(rect, heldItem->position))
          {
            character.queue_play({.animation = eatArea.animation, .speedMultiplier = character.eatSpeed});

            if (character.playedEventID == eatArea.eventID)
            {
              heldItem->chewCount++;
              character.consume_played_event();

              auto chewAnimation = schema.animations.chew + std::to_string(heldItem->chewCount);
              auto animationIndex = heldItem->chewCount > 0 ? heldItem->animationMap[chewAnimation] : -1;

              heldItem->play(animationIndex, entity::Actor::SET);

              character.calories += caloriesChew;
              character.totalCaloriesConsumed += caloriesChew;

              if (item.eatSpeedBonus.has_value())
              {
                character.eatSpeed += *item.eatSpeedBonus / (chewCountMax + 1);
                character.eatSpeed =
                    glm::clamp(character.data.eatSpeedMin, character.eatSpeed, character.data.eatSpeedMax);
              }
              if (item.digestionBonus.has_value())
              {
                character.digestionRate += *item.digestionBonus / (chewCountMax + 1);
                character.digestionRate = glm::clamp(character.data.digestionRateMin, character.digestionRate,
                                                     character.data.digestionRateMax);
              }

              if (heldItem->chewCount > chewCountMax)
              {
                isQueueFinishFood = true;
                character.totalFoodItemsEaten++;
                queuedRemoveItemIndex = heldItemIndex;
                heldItemIndex = -1;
              }
            }
          }

          if (isMouseLeftReleased)
          {
            if (fabs(delta.x) >= THROW_THRESHOLD || fabs(delta.y) >= THROW_THRESHOLD)
            {
              cursorSchema.sounds.throw_.play();
              text.set(dialogue.get(dialogue.throw_), character);
              isJustItemThrown = true;
            }
            else
              cursorSchema.sounds.release.play();

            heldItem->velocity -= delta;
            heldItemIndex = -1;
            isJustItemHeldStopped = true;
          }

          // Food stolen
          if (auto animation = character.animation_get(character.animation_name_convert(eatArea.animation));
              character.is_playing(animation->name))
          {
            if (!math::is_point_in_rectf(rect, heldItem->position))
              text.set(dialogue.get(isOverCapacity ? dialogue.foodTakenFull : dialogue.foodTaken), character);
          }
        }
      }
    }

    if (auto animation = character.animation_get(); character.time >= animation->frameNum && isQueueFinishFood)
    {
      text.set(dialogue.get(isOverCapacity ? dialogue.eatFull : dialogue.eat), character);
      isQueueFinishFood = false;
    }

    if (queuedRemoveItemIndex > -1)
    {
      items.erase(items.begin() + queuedRemoveItemIndex);
      queuedRemoveItemIndex = -1;
    }

    int heldItemMoveIndex = -1;
    for (int i = 0; i < (int)items.size(); i++)
    {
      auto& item = items[i];
      auto& schemaItem = schema.items[item.schemaID];
      auto& rotationOverride = item.overrides[item.rotationOverrideID];
      auto& rotation = *rotationOverride.frame.rotation;
      auto& gravity = schemaItem.gravity.has_value() ? *schemaItem.gravity : area.gravity;

      item.update();

      if (math::is_point_in_rectf(item.rect(), cursorPosition) && !isImguiCaptureMouse)
      {
        isItemHovered = true;
        if (auto animation = cursorSchema.animations.hover.get()) cursor.queue_play({*animation});
        cursor.state = entity::Cursor::HOVER;

        if (isMouseLeftClicked)
        {
          cursorSchema.sounds.grab.play();
          isJustItemHeld = true;
        }

        if (isMouseLeftDown)
        {
          isItemHeld = true;
          if (auto animation = cursorSchema.animations.grab.get()) cursor.queue_play({*animation});
          cursor.state = entity::Cursor::ACTION;
          heldItemIndex = i;
          heldItemMoveIndex = i;
        }

        if (isMouseRightClicked)
        {
          if (item.chewCount > 0)
            schema.sounds.dispose.play();
          else
          {
            schema.sounds.return_.play();
            returnItemIDs.emplace_back(item.schemaID);
          }

          if (heldItemIndex == i) heldItemIndex = -1;
          if (heldItemMoveIndex == i) heldItemMoveIndex = -1;
          if (heldItemIndex > i) heldItemIndex--;
          if (heldItemMoveIndex > i) heldItemMoveIndex--;
          items.erase(items.begin() + i);
          continue;
        }
      }

      if (i != heldItemIndex)
      {

        if (item.position.x <= bounds.x || item.position.x >= bounds.z)
        {
          if (item.position.x <= bounds.x) item.position.x = bounds.x + 1.0f;
          if (item.position.x >= bounds.z) item.position.x = bounds.z - 1.0f;

          item.velocity.x *= friction;
          item.velocity.x = -item.velocity.x;
          item.angularVelocity *= friction;
          item.angularVelocity = -item.angularVelocity;
          rotation = -rotation;
          schema.sounds.bounce.play();
        }

        if (item.position.y <= bounds.y || item.position.y >= bounds.w)
        {

          if (item.position.y >= bounds.w && item.velocity.y <= gravity)
          {
            item.position.y = bounds.w;
            item.velocity.y = 0;
            item.velocity.x *= friction;

            rotation = std::fmod(rotation, 360.0f);
            if (rotation < 0.0f) rotation += 360.0f;
            if (rotation > 180.0f) rotation -= 360.0f;

            auto returnStep = ROTATION_RETURN_SPEED * io.DeltaTime;
            if (std::abs(rotation) <= returnStep)
              rotation = 0.0f;
            else
              rotation += (rotation > 0.0f) ? -returnStep : returnStep;

            item.angularVelocity *= ROTATION_GROUND_DAMPING;
          }
          else
          {
            item.velocity.y = -item.velocity.y;
            item.angularVelocity *= friction;
            schema.sounds.bounce.play();
          }

          item.velocity.y *= friction;
        }

        item.velocity.x *= airResistance;
        item.velocity.y += gravity;
      }

      item.position.x = glm::clamp(bounds.x, item.position.x, bounds.z);
      item.position.y = glm::clamp(bounds.y, item.position.y, bounds.w);
    }

    if (heldItemMoveIndex != -1 && heldItemMoveIndex < (int)items.size() - 1)
    {
      auto heldItem = std::move(items[heldItemMoveIndex]);
      items.erase(items.begin() + heldItemMoveIndex);
      items.push_back(std::move(heldItem));
      heldItemIndex = (int)items.size() - 1;
    }

    cursorPositionPrevious = cursorPosition;
    cursorDeltaPrevious = cursorDelta;
  }
}
