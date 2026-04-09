#include "orbit.hpp"

#include "../../../../util/imgui.hpp"
#include "../../../../util/imgui/widget.hpp"
#include "../../../../util/math.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <glm/gtc/constants.hpp>
#include <imgui.h>

using namespace game::util::imgui;
using namespace game::resource::xml;
using namespace game::util;
using namespace glm;

namespace game::state::play::menu::arcade
{
  namespace
  {
    enum SpawnSide
    {
      TOP,
      RIGHT,
      BOTTOM,
      LEFT
    };

    bool is_rect_overlapping(const glm::vec4& left, const glm::vec4& right)
    {
      return left.x < right.x + right.z && left.x + left.z > right.x && left.y < right.y + right.w &&
             left.y + left.w > right.y;
    }

    void target_tick(Orbit::Entity& entity, const glm::vec2& target, float acceleration)
    {
      auto delta = target - entity.position;
      auto distance = glm::length(delta);

      if (distance <= 0.001f)
      {
        entity.position = target;
        entity.velocity *= 0.5f;
        if (glm::length(entity.velocity) <= 0.001f) entity.velocity = {};
        return;
      }

      auto maxSpeed = std::max(acceleration * 8.0f, 1.0f);
      auto desiredVelocity = glm::normalize(delta) * std::min(distance * 0.35f, maxSpeed);
      auto steering = desiredVelocity - entity.velocity;
      auto steeringLength = glm::length(steering);
      if (steeringLength > acceleration) steering = (steering / steeringLength) * acceleration;

      entity.velocity += steering;

      auto velocityLength = glm::length(entity.velocity);
      if (velocityLength > maxSpeed) entity.velocity = (entity.velocity / velocityLength) * maxSpeed;

      entity.position += entity.velocity;

      if (glm::distance(entity.position, target) <= maxSpeed)
      {
        entity.position = glm::mix(entity.position, target, 0.15f);
      }
    }

    void follower_angles_refresh(std::vector<Orbit::Entity>& entities)
    {
      std::vector<Orbit::Entity*> followers{};
      for (auto& entity : entities)
        if (entity.type == Orbit::Entity::FOLLOWER) followers.emplace_back(&entity);

      if (followers.empty()) return;

      std::sort(followers.begin(), followers.end(),
                [](const Orbit::Entity* left, const Orbit::Entity* right) { return left->colorID < right->colorID; });

      auto baseAngle = followers.front()->orbitAngle;
      auto spacing = glm::two_pi<float>() / (float)followers.size();

      for (int i = 0; i < (int)followers.size(); i++)
        followers[i]->orbitAngle = baseAngle + spacing * (float)i;
    }

    const glm::vec3* color_value_get(const resource::xml::Orbit& schema, int colorID)
    {
      if (colorID < 0 || colorID >= (int)schema.colors.size()) return nullptr;
      return &schema.colors[colorID].value;
    }

    int random_available_color_get(const resource::xml::Orbit& schema, int level)
    {
      auto availableCount = std::min(level, (int)schema.colors.size());
      if (availableCount <= 0) return -1;
      return (int)math::random_max((float)availableCount);
    }

    int unlocked_level_get(const resource::xml::Orbit& schema, int score)
    {
      int unlockedLevel = 0;

      for (auto& color : schema.colors)
      {
        if (score >= color.scoreThreshold)
          unlockedLevel++;
        else
          break;
      }

      return std::max(1, unlockedLevel);
    }

    void color_override_set(Orbit::Entity& entity, const resource::xml::Orbit& schema, int colorID,
                            const std::string& layerName)
    {
      auto color = color_value_get(schema, colorID);
      if (!color) return;
      if (!entity.layerMap.contains(layerName)) return;

      entity::Actor::Override override_{entity.layerMap.at(layerName), Anm2::LAYER, entity::Actor::Override::SET};
      override_.frame.tint.x = color->r;
      override_.frame.tint.y = color->g;
      override_.frame.tint.z = color->b;
      entity.overrides.emplace_back(std::move(override_));
    }

    void idle_queue(Orbit::Entity& entity)
    {
      if (!entity.animationIdle.empty())
        entity.queue_play({.animation = entity.animationIdle, .isPlayAfterAnimation = true});
    }

    void spawn_animation_play(Orbit::Entity& entity)
    {
      if (!entity.animationSpawn.empty())
      {
        entity.play(entity.animationSpawn, entity::Actor::PLAY_FORCE);
        idle_queue(entity);
      }
    }
  }

  void Orbit::spawn(entity::Character& character, Orbit::Entity::Type type, int colorID)
  {
    auto& schema = character.data.orbitSchema;

    switch (type)
    {
      case Entity::PLAYER:
        if (!schema.player.anm2.is_valid()) return;
        entities.emplace_back(schema.player.anm2, Entity::PLAYER);
        entities.back().position = centerPosition;
        entities.back().animationIdle = schema.player.animations.idle;
        entities.back().animationSpawn = schema.player.animations.spawn;
        entities.back().animationDeath = schema.player.animations.death;
        entities.back().hitboxNull = schema.player.hitboxNull;
        spawn_animation_play(entities.back());
        return;
      case Entity::FOLLOWER:
      {
        if (!schema.follower.anm2.is_valid()) return;
        if (colorID < 0 || colorID >= (int)schema.colors.size()) return;

        Entity follower{schema.follower.anm2, Entity::FOLLOWER};
        follower.colorID = colorID;
        follower.orbitAngle = 0.0f;
        follower.position = centerPosition;
        follower.animationIdle = schema.follower.animations.idle;
        follower.animationSpawn = schema.follower.animations.spawn;
        follower.animationDeath = schema.follower.animations.death;
        follower.hitboxNull = schema.follower.hitboxNull;
        color_override_set(follower, schema, colorID, schema.follower.overrideTintLayer);
        spawn_animation_play(follower);

        entities.emplace_back(std::move(follower));
        follower_angles_refresh(entities);
        return;
      }
      case Entity::ENEMY:
      {
        if (!schema.enemy.anm2.is_valid()) return;

        Entity enemy{schema.enemy.anm2, Entity::ENEMY};
        enemy.colorID = colorID;
        enemy.animationIdle = schema.enemy.animations.idle;
        enemy.animationSpawn = schema.enemy.animations.spawn;
        enemy.animationDeath = schema.enemy.animations.death;
        enemy.hitboxNull = schema.enemy.hitboxNull;
        color_override_set(enemy, schema, colorID, schema.enemy.overrideTintLayer);
        spawn_animation_play(enemy);

        auto rect = enemy.rect();
        auto width = rect.z;
        auto height = rect.w;
        auto side = (SpawnSide)math::random_max(4.0f);
        switch (side)
        {
          case TOP:
            enemy.position = vec2(math::random_max(canvas.size.x), -height - schema.enemy.spawnPadding);
            break;
          case RIGHT:
            enemy.position = vec2(canvas.size.x + width + schema.enemy.spawnPadding, math::random_max(canvas.size.y));
            break;
          case BOTTOM:
            enemy.position = vec2(math::random_max(canvas.size.x), canvas.size.y + height + schema.enemy.spawnPadding);
            break;
          case LEFT:
            enemy.position = vec2(-width - schema.enemy.spawnPadding, math::random_max(canvas.size.y));
            break;
        }

        entities.emplace_back(std::move(enemy));

        if (schema.warning.anm2.is_valid())
        {
          Entity warning{schema.warning.anm2, Entity::WARNING};
          warning.colorID = colorID;
          color_override_set(warning, schema, colorID, schema.warning.overrideTintLayer);

          auto warningRect = warning.rect();
          auto warningWidth = warningRect.z;
          auto warningHeight = warningRect.w;

          switch (side)
          {
            case TOP:
              warning.position = vec2(glm::clamp(entities.back().position.x, warningWidth * 0.5f,
                                                 (float)canvas.size.x - warningWidth * 0.5f),
                                      warningHeight * 0.5f);
              break;
            case RIGHT:
              warning.position = vec2((float)canvas.size.x - warningWidth * 0.5f,
                                      glm::clamp(entities.back().position.y, warningHeight * 0.5f,
                                                 (float)canvas.size.y - warningHeight * 0.5f));
              break;
            case BOTTOM:
              warning.position = vec2(glm::clamp(entities.back().position.x, warningWidth * 0.5f,
                                                 (float)canvas.size.x - warningWidth * 0.5f),
                                      (float)canvas.size.y - warningHeight * 0.5f);
              break;
            case LEFT:
              warning.position = vec2(warningWidth * 0.5f, glm::clamp(entities.back().position.y, warningHeight * 0.5f,
                                                                      (float)canvas.size.y - warningHeight * 0.5f));
              break;
          }

          entities.emplace_back(std::move(warning));
        }
        return;
      }
      case Entity::WARNING:
      default:
        return;
    }
  }

  void Orbit::reset(entity::Character& character)
  {
    entities.clear();
    sounds = &character.data.orbitSchema.sounds;
    cursorPosition = {};
    centerPosition = {};
    level = 0;
    highScoreAtRunStart = highScore;
    score = 0;
    isHighScoreAchievedThisRun = false;
    itemEffectManager = {};
    followerRadius = {};
    playerTargetAcceleration = {};
    followerTargetAcceleration = {};
    playerTimeAfterHurt = {};
    enemySpeed = {};
    enemySpeedScoreBonus = {};
    enemySpeedGainBase = {};
    enemySpeedGainScoreBonus = {};
    rotationSpeed = {};
    rotationSpeedMax = {};
    rotationSpeedFriction = {};
    startTimer = character.data.orbitSchema.startTime;
    hurtTimer = 0;
    isPlayerDying = false;
    isRotateLeft = false;
    isRotateRight = false;
  }

  void Orbit::tick()
  {
    for (auto& entity : entities)
      entity.tick();

    itemEffectManager.tick();
    canvas.tick();
  }

  bool Orbit::update(Resources& resources, entity::Character& character, entity::Cursor& cursor, Inventory& inventory,
                     Text& text, menu::Toasts& toasts)
  {
    auto& strings = character.data.strings;
    auto& schema = character.data.orbitSchema;
    sounds = &schema.sounds;
    auto& style = ImGui::GetStyle();
    auto drawList = ImGui::GetWindowDrawList();
    auto& textureShader = resources.shaders[resource::shader::TEXTURE];
    auto& rectShader = resources.shaders[resource::shader::RECT];
    ImGui::Text(strings.get(Strings::ArcadeScoreFormat).c_str(), score);
    auto bestText = std::vformat(strings.get(Strings::ArcadeBestScoreFormat), std::make_format_args(highScore));
    auto cursorPos = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(bestText.c_str()).x,
                               cursorPos.y - ImGui::GetTextLineHeightWithSpacing()));
    ImGui::Text(strings.get(Strings::ArcadeBestScoreFormat).c_str(), highScore);
    auto padding = ImGui::GetTextLineHeightWithSpacing();
    auto contentRegionAvail = ImGui::GetContentRegionAvail();
    auto contentRegionPosition = ImGui::GetCursorScreenPos();
    auto contentBounds =
        ImVec4(contentRegionPosition.x, contentRegionPosition.y, contentRegionAvail.x, contentRegionAvail.y);
    auto available =
        imgui::to_vec2(contentRegionAvail) - vec2(0.0f, ImGui::GetFrameHeightWithSpacing() + style.WindowPadding.y);
    auto canvasSize = glm::max(vec2(1.0f), available - vec2(padding * 2.0f));
    auto canvasScreenPosition = imgui::to_vec2(ImGui::GetCursorScreenPos()) + vec2(padding);
    centerPosition = canvasSize * 0.5f;

    if (isPlayerDying)
    {
      Entity* playerEntity = nullptr;
      for (auto& entity : entities)
        if (entity.type == Entity::PLAYER)
        {
          playerEntity = &entity;
          break;
        }

      if (!playerEntity || playerEntity->state == entity::Actor::STOPPED)
      {
        reset(character);
      }
    }

    if (entities.empty() && startTimer <= 0) startTimer = schema.startTime;

    if (entities.empty()) spawn(character, Entity::PLAYER);

    followerRadius = schema.player.followerRadius;
    playerTargetAcceleration = schema.player.targetAcceleration;
    followerTargetAcceleration = schema.follower.targetAcceleration;
    playerTimeAfterHurt = schema.player.timeAfterHurt;
    enemySpeed = schema.enemy.speed;
    enemySpeedScoreBonus = schema.enemy.speedScoreBonus;
    enemySpeedGainBase = schema.enemy.speedGainBase;
    enemySpeedGainScoreBonus = schema.enemy.speedGainScoreBonus;
    rotationSpeed = schema.player.rotationSpeed;
    rotationSpeedMax = schema.player.rotationSpeedMax;
    rotationSpeedFriction = schema.player.rotationSpeedFriction;
    auto nextLevel = std::min(unlocked_level_get(schema, score), (int)schema.colors.size());
    if (nextLevel > level)
    {
      schema.sounds.levelUp.play();

      auto colorIndex = nextLevel - 1;
      if (colorIndex >= 0 && colorIndex < (int)schema.colors.size())
      {
        auto& pool = schema.colors[colorIndex].pool;
        if (pool.is_valid() && text.is_interruptible()) text.set(character.data.dialogue.get(pool), character);
      }
    }
    level = nextLevel;

    auto player_get = [&]() -> Entity*
    {
      for (auto& entity : entities)
        if (entity.type == Entity::PLAYER) return &entity;

      return nullptr;
    };

    auto player = player_get();

    if (player)
    {
      auto desiredFollowerCount = std::min(level, (int)schema.colors.size());
      auto currentFollowerCount = (int)std::count_if(entities.begin(), entities.end(), [](const Entity& entity)
                                                     { return entity.type == Entity::FOLLOWER; });
      auto currentEnemyCount = (int)std::count_if(entities.begin(), entities.end(),
                                                  [](const Entity& entity) { return entity.type == Entity::ENEMY; });

      if (currentFollowerCount != desiredFollowerCount)
      {
        entities.erase(std::remove_if(entities.begin(), entities.end(),
                                      [](const Entity& entity) { return entity.type == Entity::FOLLOWER; }),
                       entities.end());

        for (int i = 0; i < desiredFollowerCount; i++)
          spawn(character, Entity::FOLLOWER, i);

        player = player_get();
      }

      if (startTimer <= 0 && hurtTimer <= 0 && !isPlayerDying && !schema.colors.empty())
      {
        auto colorID = random_available_color_get(schema, level);
        if (colorID == -1) return false;

        if (currentEnemyCount == 0)
        {
          spawn(character, Entity::ENEMY, colorID);
          player = player_get();
        }

        auto spawnChance = schema.enemy.spawnChanceBase + schema.enemy.spawnChanceScoreBonus * (float)score;
        if (math::random_percent_roll(spawnChance))
        {
          colorID = random_available_color_get(schema, level);
          if (colorID == -1) return false;
          spawn(character, Entity::ENEMY, colorID);
          player = player_get();
        }
      }
    }

    auto mousePosition = imgui::to_vec2(ImGui::GetMousePos());
    auto canvasBoundsMax = canvasScreenPosition + canvasSize;
    auto isHoveringCanvas = mousePosition.x >= canvasScreenPosition.x && mousePosition.x <= canvasBoundsMax.x &&
                            mousePosition.y >= canvasScreenPosition.y && mousePosition.y <= canvasBoundsMax.y;

    cursor.isVisible = !isHoveringCanvas;
    isRotateLeft = startTimer <= 0 && hurtTimer <= 0 && !isPlayerDying && isHoveringCanvas &&
                   ImGui::IsMouseDown(ImGuiMouseButton_Left);
    isRotateRight = startTimer <= 0 && hurtTimer <= 0 && !isPlayerDying && isHoveringCanvas &&
                    ImGui::IsMouseDown(ImGuiMouseButton_Right);
    cursorPosition = glm::clamp(mousePosition - canvasScreenPosition, vec2(0.0f), canvasSize);

    if (player)
    {
      if (isPlayerDying || hurtTimer > 0) player->velocity *= 0.85f;

      if (!isPlayerDying && hurtTimer <= 0 && isRotateLeft) player->rotationVelocity -= rotationSpeed;
      if (!isPlayerDying && hurtTimer <= 0 && isRotateRight) player->rotationVelocity += rotationSpeed;
      player->rotationVelocity = glm::clamp(player->rotationVelocity, -rotationSpeedMax, rotationSpeedMax);
      player->rotationVelocity *= rotationSpeedFriction;

      if (!isPlayerDying && hurtTimer <= 0)
        target_tick(*player, startTimer > 0 ? centerPosition : cursorPosition, playerTargetAcceleration);
    }

    for (auto& entity : entities)
    {
      switch (entity.type)
      {
        case Entity::FOLLOWER:
          if (player)
          {
            entity.orbitAngle += player->rotationVelocity;

            auto radius = std::max(0.0f, followerRadius);
            auto target = player->position + vec2(std::cos(entity.orbitAngle), std::sin(entity.orbitAngle)) * radius;
            target_tick(entity, target, followerTargetAcceleration);
          }
          break;
        case Entity::ENEMY:
          if (player && !entity.isMarkedForRemoval)
          {
            auto delta = player->position - entity.position;
            auto distance = glm::length(delta);
            auto speed = (enemySpeed + enemySpeedScoreBonus * (float)score) +
                         (enemySpeedGainBase + enemySpeedGainScoreBonus * (float)score);
            if (distance > 0.001f) entity.position += glm::normalize(delta) * speed;
          }
          break;
        default:
          break;
      }
    }

    for (auto& follower : entities)
    {
      if (isPlayerDying) break;
      if (follower.type != Entity::FOLLOWER) continue;
      if (follower.hitboxNull.empty() || !follower.nullMap.contains(follower.hitboxNull)) continue;

      auto followerRect = follower.null_frame_rect(follower.nullMap.at(follower.hitboxNull));
      if (std::isnan(followerRect.x)) continue;

      for (auto& enemy : entities)
      {
        if (enemy.type != Entity::ENEMY || enemy.isMarkedForRemoval) continue;
        if (enemy.colorID != follower.colorID) continue;
        if (enemy.hitboxNull.empty() || !enemy.nullMap.contains(enemy.hitboxNull)) continue;

        auto enemyRect = enemy.null_frame_rect(enemy.nullMap.at(enemy.hitboxNull));
        if (std::isnan(enemyRect.x)) continue;
        if (!is_rect_overlapping(followerRect, enemyRect)) continue;

        enemy.isMarkedForRemoval = true;
        if (!enemy.animationDeath.empty())
          enemy.play(enemy.animationDeath, entity::Actor::PLAY_FORCE);
        else
          enemy.state = entity::Actor::STOPPED;

        spawn_animation_play(follower);
        score++;
        auto rewardChance = schema.rewardChanceBase + (schema.rewardChanceScoreBonus * score);
        auto rewardRollCount = schema.rewardRollChanceBase + (schema.rewardRollScoreBonus * score);
        itemRewards.reward_random_items_try(inventory, itemEffectManager, character.data.itemSchema, contentBounds,
                                            rewardChance, rewardRollCount, menu::ItemEffectManager::SHOOT_UP);
        if (score > highScore)
        {
          highScore = score;

          if (!isHighScoreAchievedThisRun && highScoreAtRunStart > 0)
          {
            isHighScoreAchievedThisRun = true;
            schema.sounds.highScore.play();
            auto toastText = strings.get(Strings::ArcadeHighScoreToast);
            auto toastPosition = imgui::to_imvec2(
                canvasScreenPosition + player->position -
                vec2(ImGui::CalcTextSize(toastText.c_str()).x * 0.5f, ImGui::GetTextLineHeightWithSpacing() * 2.0f));
            toasts.spawn(toastText, toastPosition, 60);
          }
        }
      }
    }

    if (player && !isPlayerDying && hurtTimer <= 0 && !player->hitboxNull.empty() &&
        player->nullMap.contains(player->hitboxNull))
    {
      auto playerRect = player->null_frame_rect(player->nullMap.at(player->hitboxNull));

      if (!std::isnan(playerRect.x))
      {
        auto isHit = false;

        for (auto& enemy : entities)
        {
          if (enemy.type != Entity::ENEMY || enemy.isMarkedForRemoval) continue;
          if (enemy.hitboxNull.empty() || !enemy.nullMap.contains(enemy.hitboxNull)) continue;

          auto enemyRect = enemy.null_frame_rect(enemy.nullMap.at(enemy.hitboxNull));
          if (std::isnan(enemyRect.x)) continue;
          if (!is_rect_overlapping(playerRect, enemyRect)) continue;

          isHit = true;
          break;
        }

        if (isHit)
        {
          if (sounds) sounds->hurt.play();
          if (isHighScoreAchievedThisRun) schema.sounds.highScoreLoss.play();
          if (schema.poolDeath.is_valid() && text.is_interruptible())
            text.set(character.data.dialogue.get(schema.poolDeath), character);
          hurtTimer = playerTimeAfterHurt;
          isPlayerDying = true;
          player->velocity = {};
          player->rotationVelocity = 0.0f;
          if (!player->animationDeath.empty())
            player->play(player->animationDeath, entity::Actor::PLAY_FORCE);
          else
            player->state = entity::Actor::STOPPED;

          entities.erase(std::remove_if(entities.begin(), entities.end(),
                                        [](const Entity& entity)
                                        {
                                          return entity.type == Entity::ENEMY || entity.type == Entity::WARNING ||
                                                 entity.type == Entity::FOLLOWER;
                                        }),
                         entities.end());
        }
      }
    }

    entities.erase(std::remove_if(entities.begin(), entities.end(),
                                  [](const Entity& entity)
                                  {
                                    return (entity.type == Entity::WARNING && entity.state == entity::Actor::STOPPED) ||
                                           (entity.type == Entity::ENEMY && entity.isMarkedForRemoval &&
                                            entity.state == entity::Actor::STOPPED);
                                  }),
                   entities.end());

    if (startTimer > 0) startTimer--;
    if (hurtTimer > 0) hurtTimer--;

    canvas.bind();
    canvas.size_set(ivec2(canvasSize));
    canvas.clear(color::BLACK);

    for (auto& entity : entities)
      if (entity.type == Entity::PLAYER || entity.type == Entity::FOLLOWER || entity.type == Entity::ENEMY ||
          entity.type == Entity::WARNING)
        entity.render(textureShader, rectShader, canvas);

    canvas.unbind();

    ImGui::Dummy(ImVec2(0, padding));
    ImGui::SetCursorScreenPos(imgui::to_imvec2(canvasScreenPosition));
    ImGui::Image(canvas.texture, imgui::to_imvec2(canvasSize));
    itemEffectManager.render(resources, character.data.itemSchema, contentBounds, ImGui::GetIO().DeltaTime);
    ImGui::Dummy(ImVec2(0, padding));
    toasts.update(drawList);

    auto isMenuPressed = WIDGET_FX(ImGui::Button(strings.get(Strings::ArcadeMenuBackButton).c_str()));
    if (ImGui::IsItemHovered()) ImGui::SetItemTooltip("%s", strings.get(Strings::ArcadeMenuBackButtonTooltip).c_str());
    return isMenuPressed;
  }
}
