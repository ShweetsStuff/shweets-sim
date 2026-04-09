#include "item_effect_manager.hpp"

#include "../../../util/math.hpp"

using namespace game::util;
using namespace game::resource;
using namespace glm;

namespace game::state::play::menu
{
  void ItemEffectManager::tick()
  {
    for (auto& [i, actor] : actors)
      actor.tick();
  }

  void ItemEffectManager::spawn(int itemID, const resource::xml::Item& itemSchema, const ImVec4& bounds, Mode mode)
  {
    static constexpr auto ITEM_SHOOT_UP_HORIZONTAL_SPEED_MIN = -250.0f;
    static constexpr auto ITEM_SHOOT_UP_HORIZONTAL_SPEED_MAX = 250.0f;
    static constexpr auto ITEM_SHOOT_UP_VERTICAL_SPEED_MIN = 500.0f;
    static constexpr auto ITEM_SHOOT_UP_VERTICAL_SPEED_MAX = 1000.0f;
    static constexpr auto ITEM_ROTATION_VELOCITY_MIN = -45.0f;
    static constexpr auto ITEM_ROTATION_VELOCITY_MAX = 45.0f;

    if (!actors.contains(itemID))
    {
      actors[itemID] = entity::Actor(itemSchema.anm2s[itemID], {}, entity::Actor::SET);
      rects[itemID] = actors[itemID].rect();
    }

    auto size = ImVec2(bounds.z, bounds.w);
    auto rect = rects[itemID];
    auto rectSize = vec2(rect.z, rect.w);
    auto previewScale = (rectSize.x <= 0.0f || rectSize.y <= 0.0f || size.x <= 0.0f || size.y <= 0.0f ||
                         !std::isfinite(rectSize.x) || !std::isfinite(rectSize.y))
                            ? 0.0f
                            : std::min(size.x / rectSize.x, size.y / rectSize.y);
    previewScale = std::min(1.0f, previewScale);
    auto previewSize = rectSize * previewScale;
    auto minX = 0.0f;
    auto maxX = size.x - previewSize.x;
    auto spawnX = minX >= maxX ? 0.0f : math::random_in_range(minX, maxX);
    auto rotationVelocity = math::random_in_range(ITEM_ROTATION_VELOCITY_MIN, ITEM_ROTATION_VELOCITY_MAX);

    Entry entry{};
    entry.id = itemID;
    entry.mode = mode;
    entry.rotationVelocity = rotationVelocity;

    switch (mode)
    {
      case SHOOT_UP:
        entry.position = ImVec2(spawnX, std::max(0.0f, size.y - previewSize.y));
        entry.velocity.x =
            math::random_in_range(ITEM_SHOOT_UP_HORIZONTAL_SPEED_MIN, ITEM_SHOOT_UP_HORIZONTAL_SPEED_MAX);
        entry.velocity.y = -math::random_in_range(ITEM_SHOOT_UP_VERTICAL_SPEED_MIN, ITEM_SHOOT_UP_VERTICAL_SPEED_MAX);
        break;
      case FALL_DOWN:
      default:
        entry.position = ImVec2(spawnX, -previewSize.y - math::random_in_range(0.0f, size.y));
        entry.velocity = {};
        break;
    }

    entries.emplace_back(std::move(entry));
  }

  void ItemEffectManager::render(Resources& resources, const resource::xml::Item& itemSchema, const ImVec4& bounds,
                                 float deltaTime)
  {
    static constexpr auto ITEM_FALL_GRAVITY = 2400.0f;
    auto position = ImVec2(bounds.x, bounds.y);
    auto size = ImVec2(bounds.z, bounds.w);

    auto drawList = ImGui::GetWindowDrawList();
    auto windowMin = position;
    auto windowMax = ImVec2(position.x + size.x, position.y + size.y);

    ImGui::PushClipRect(windowMin, windowMax, true);
    for (int i = 0; i < (int)entries.size(); i++)
    {
      auto& item = entries[i];
      if (!actors.contains(item.id))
      {
        entries.erase(entries.begin() + i--);
        continue;
      }

      auto rect = rects[item.id];
      auto rectSize = vec2(rect.z, rect.w);
      auto previewScale = (rectSize.x <= 0.0f || rectSize.y <= 0.0f || size.x <= 0.0f || size.y <= 0.0f ||
                           !std::isfinite(rectSize.x) || !std::isfinite(rectSize.y))
                              ? 0.0f
                              : std::min(size.x / rectSize.x, size.y / rectSize.y);
      previewScale = std::min(1.0f, previewScale);
      auto previewSize = rectSize * previewScale;
      auto canvasSize = ivec2(std::max(1.0f, previewSize.x), std::max(1.0f, previewSize.y));

      if (!canvases.contains(item.id)) canvases.emplace(item.id, Canvas(canvasSize, Canvas::FLIP));
      auto& canvas = canvases[item.id];
      canvas.zoom = math::to_percent(previewScale);
      canvas.pan = vec2(rect.x, rect.y);
      canvas.bind();
      canvas.size_set(canvasSize);
      canvas.clear();

      actors[item.id].overrides.emplace_back(-1, resource::xml::Anm2::ROOT, entity::Actor::Override::SET,
                                             resource::xml::Anm2::FrameOptional{.rotation = item.rotation});
      actors[item.id].render(resources.shaders[shader::TEXTURE], resources.shaders[shader::RECT], canvas);
      actors[item.id].overrides.pop_back();
      canvas.unbind();

      auto min = ImVec2(position.x + item.position.x, position.y + item.position.y);
      auto max = ImVec2(item.position.x + previewSize.x, item.position.y + previewSize.y);
      max.x += position.x;
      max.y += position.y;
      drawList->AddImage(canvas.texture, min, max);

      item.rotation += item.rotationVelocity * deltaTime;
      item.position.x += item.velocity.x * deltaTime;
      item.position.y += item.velocity.y * deltaTime;

      switch (item.mode)
      {
        case SHOOT_UP:
        case FALL_DOWN:
        default:
          item.velocity.y += ITEM_FALL_GRAVITY * deltaTime;
          break;
      }

      if (item.position.y > size.y || item.position.x < -previewSize.x || item.position.x > size.x + previewSize.x)
        entries.erase(entries.begin() + i--);
    }
    ImGui::PopClipRect();
  }
}
