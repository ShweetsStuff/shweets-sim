#include "character_manager.hpp"

#include "../../util/math.hpp"

#include <imgui.h>
#include <optional>

using namespace game::resource::xml;
using namespace game::util;

namespace game::state::play
{
  void CharacterManager::update(entity::Character& character, entity::Cursor& cursor, Text& text, Canvas& canvas)
  {
    auto interact_area_override_tick = [](entity::Actor::Override& override_)
    {
      if (override_.frame.scale.has_value() && override_.frameBase.scale.has_value() && override_.time.has_value() &&
          override_.timeStart.has_value())
      {
        auto percent = glm::clamp(*override_.time / *override_.timeStart, 0.0f, 1.0f);
        auto elapsed = 1.0f - percent;

        auto oscillation = cosf(elapsed * glm::tau<float>() * override_.cycles);
        auto envelope = percent;
        auto amplitude = glm::abs(*override_.frameBase.scale);

        *override_.frame.scale = amplitude * (oscillation * envelope);
      }
    };

    auto& dialogue = character.data.dialogue;
    auto cursorWorldPosition = canvas.screen_position_convert(cursor.position);
    auto isMouseLeftClick = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    auto isMouseLeftReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
    auto isImguiCaptureMouse = ImGui::GetIO().WantCaptureMouse;

    isInteractingPrevious = isInteracting;
    isHoveringPrevious = isHovering;
    isHoldInteractingPrevious = isHoldInteracting;
    isHovering = false;
    if (!isInteracting) isHoldInteracting = false;

    if (isJustStoppedHoldInteracting)
    {
      cursor.queue_play({cursor.defaultAnimation});
      if (character.queuedPlay.empty()) character.queue_idle_animation();
      isJustStoppedHoldInteracting = false;
    }
    else if (isJustStoppedInteracting)
    {
      cursor.queue_play({cursor.defaultAnimation});
      isJustStoppedInteracting = false;
    }

    if (isJustStoppedHovering)
    {
      cursor.queue_play({cursor.defaultAnimation});
      isJustStoppedHovering = false;
    }

    for (int i = 0; i < (int)character.data.interactAreas.size(); i++)
    {
      auto& interactArea = character.data.interactAreas.at(i);
      if (interactArea.nullID == -1) continue;
      auto rect = character.null_frame_rect(interactArea.nullID);

      if (cursor.state == entity::Cursor::DEFAULT && math::is_point_in_rectf(rect, cursorWorldPosition) &&
          !isImguiCaptureMouse && interactArea.typeID == cursor.interactTypeID)
      {
        cursor.state = entity::Cursor::HOVER;
        cursor.queue_play({interactArea.animationCursorHover});
        isHovering = true;
        interactAreaID = i;

        if (isMouseLeftClick)
        {
          isInteracting = true;
          isHoldInteracting = interactArea.isHold;
          interactArea.sound.play();

          if (interactArea.digestionBonusClick > 0 && character.calories > 0 && !character.isDigesting)
            character.digestionProgress += interactArea.digestionBonusClick;

          if (interactArea.layerID != -1)
          {
            character.overrides.emplace_back(entity::Actor::Override(
                interactArea.layerID, Anm2::LAYER, entity::Actor::Override::ADD,
                {.scale = glm::vec2(interactArea.scaleEffectAmplitude)}, std::optional<float>(interactArea.time),
                interact_area_override_tick, interactArea.scaleEffectCycles));
          }

          if (text.is_interruptible())
          {
            auto& pool = character.is_over_capacity() && interactArea.poolFull.is_valid() ? interactArea.poolFull
                                                                                           : interactArea.pool;
            if (pool.is_valid())
              text.set(dialogue.get(pool), character);
          }
        }

        if (isInteracting)
        {
          cursor.state = entity::Cursor::ACTION;
          cursor.queue_play({interactArea.animationCursorActive});

          if (interactArea.digestionBonusRub > 0 && character.calories > 0 && !character.isDigesting)
          {
            auto mouseDelta = cursorWorldPosition - cursorWorldPositionPrevious;
            auto digestionBonus = (fabs(mouseDelta.x) + fabs(mouseDelta.y)) * interactArea.digestionBonusRub;
            character.digestionProgress += digestionBonus;
          }
        }
      }

      if ((i == interactAreaID && !math::is_point_in_rectf(rect, cursorWorldPosition)) || isMouseLeftReleased ||
          isImguiCaptureMouse)
      {
        isInteracting = false;
        isHoldInteracting = false;
        interactAreaID = -1;
      }
    }

    if (isInteracting != isInteractingPrevious && !isInteracting) isJustStoppedInteracting = true;
    if (isHoldInteracting != isHoldInteractingPrevious && !isHoldInteracting) isJustStoppedHoldInteracting = true;
    if (isHovering != isHoveringPrevious && !isHovering) isJustStoppedHovering = true;

    cursorWorldPositionPrevious = cursorWorldPosition;

    if (character.isJustDigested && text.is_interruptible()) text.set(dialogue.get(dialogue.digest), character);
    if (character.isJustStageUp) text.set(dialogue.get(dialogue.stageUp), character);
  }
}
