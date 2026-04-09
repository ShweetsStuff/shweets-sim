#pragma once

#include "../../../../entity/actor.hpp"
#include "../../../../entity/cursor.hpp"
#include "../../../../resources.hpp"

#include "../../../../util/color.hpp"
#include "../inventory.hpp"
#include "../item_effect_manager.hpp"
#include "../../text.hpp"
#include "../toasts.hpp"
#include "../../item/reward.hpp"

namespace game::state::play::menu::arcade
{
  class Orbit
  {
  public:
    class Entity : public entity::Actor
    {
    public:
      enum Type
      {
        PLAYER,
        FOLLOWER,
        ENEMY,
        WARNING
      };

      Type type{PLAYER};
      glm::vec2 velocity{};
      float rotationVelocity{};
      std::string animationIdle{};
      std::string animationSpawn{};
      std::string animationDeath{};
      std::string hitboxNull{"Hitbox"};
      bool isMarkedForRemoval{};
      int health{3};
      int colorID{};
      float orbitAngle{};

      Entity() = default;
      Entity(resource::xml::Anm2 anm2, Type type = PLAYER) : entity::Actor(std::move(anm2)), type(type) {}
    };

    std::vector<Entity> entities{};
    resource::xml::Orbit::Sounds* sounds{};
    Canvas canvas{{1, 1}};
    glm::vec2 cursorPosition{};
    glm::vec2 centerPosition{};
    int level{1};
    int score{};
    int highScore{};
    int highScoreAtRunStart{};
    bool isHighScoreAchievedThisRun{};
    menu::ItemEffectManager itemEffectManager{};
    game::state::play::item::Reward itemRewards{};
    float followerRadius{};
    float playerTargetAcceleration{};
    float followerTargetAcceleration{};
    int playerTimeAfterHurt{};
    float enemySpeed{};
    float enemySpeedScoreBonus{};
    float enemySpeedGainBase{};
    float enemySpeedGainScoreBonus{};
    float rotationSpeed{};
    float rotationSpeedMax{};
    float rotationSpeedFriction{};
    int startTimer{};
    int hurtTimer{};
    bool isPlayerDying{};
    bool isRotateLeft{};
    bool isRotateRight{};

    Orbit() = default;
    void reset(entity::Character&);
    void tick();
    void spawn(entity::Character&, Entity::Type, int colorID = -1);
    bool update(Resources&, entity::Character&, entity::Cursor&, Inventory& inventory, Text& text, menu::Toasts&);
  };
}
