#include "character.hpp"

#include <format>

#include "../util/math.hpp"
#include "../util/vector.hpp"

using namespace game::util;
using namespace glm;

namespace game::entity
{
  Character::Character(const Character&) = default;
  Character::Character(Character&&) noexcept = default;
  Character& Character::operator=(const Character&) = default;
  Character& Character::operator=(Character&&) noexcept = default;

  Character::Character(resource::xml::Character& _data, glm::ivec2 _position) : Actor(_data.anm2, _position)
  {
    data = _data;

    auto& save = data.save;
    auto saveIsValid = save.is_valid();

    capacity = saveIsValid ? save.capacity : (double)data.capacity;
    weight = saveIsValid ? save.weight : (double)data.weight;
    digestionRate = saveIsValid ? save.digestionRate : (double)data.digestionRate;
    eatSpeed = saveIsValid ? save.eatSpeed : (double)data.eatSpeed;

    calories = saveIsValid ? save.calories : 0;

    isDigesting = saveIsValid ? save.isDigesting : false;
    digestionProgress = saveIsValid ? save.digestionProgress : 0;
    digestionTimer = saveIsValid ? save.digestionTimer : 0;

    auto& talkSource = data.talkOverride.layerSource;
    auto& talkDestination = data.talkOverride.layerDestination;
    talkOverrideID = vector::push_index(overrides, Actor::Override(talkDestination, Anm2::LAYER, Override::SET));
    for (auto& animation : animations)
    {
      if (!animation.layerAnimations.contains(talkSource))
        animationTalkDurations.emplace_back(-1);
      else
        animationTalkDurations.emplace_back(item_length(&animation.layerAnimations.at(talkSource)));
    }

    auto& blinkSource = data.blinkOverride.layerSource;
    auto& blinkDestination = data.blinkOverride.layerDestination;
    blinkOverrideID = vector::push_index(overrides, Actor::Override(blinkDestination, Anm2::LAYER, Override::SET));
    for (auto& animation : animations)
    {
      if (!animation.layerAnimations.contains(blinkSource))
        animationBlinkDurations.emplace_back(-1);
      else
        animationBlinkDurations.emplace_back(item_length(&animation.layerAnimations.at(blinkSource)));
    }

    for (int i = 0; i < (int)data.expandAreas.size(); i++)
    {
      auto& expandArea = data.expandAreas[i];
      expandAreaOverrideLayerIDs[i] =
          vector::push_index(overrides, Actor::Override(expandArea.layerID, Anm2::LAYER, Override::ADD));
      expandAreaOverrideNullIDs[i] =
          vector::push_index(overrides, Actor::Override(expandArea.nullID, Anm2::NULL_, Override::ADD));
    }

    for (int i = 0; i < (int)data.interactAreas.size(); i++)
    {
      auto& interactArea = data.interactAreas[i];
      if (interactArea.layerID != -1)
        interactAreaOverrides[i] = Actor::Override(interactArea.layerID, Anm2::LAYER, Override::ADD);
    }

    stage = stage_get();
    expand_areas_apply();
  }

  float Character::weight_get(measurement::System system)
  {
    return system == measurement::IMPERIAL ? weight * (double)measurement::KG_TO_LB : weight;
  }

  int Character::stage_from_weight_get(float checkWeight) const
  {
    if (data.stages.empty()) return 0;
    if (checkWeight <= data.weight) return 0;

    for (int i = 0; i < (int)data.stages.size(); i++)
      if (checkWeight < data.stages[i].threshold) return i;

    return stage_max_get();
  }

  int Character::stage_get() const { return stage_from_weight_get(weight); }

  int Character::stage_max_get() const { return (int)data.stages.size(); }

  float Character::stage_threshold_get(int stageIndex, measurement::System system) const
  {
    if (stageIndex == -1) stageIndex = this->stage;

    double threshold = data.weight;

    if (!data.stages.empty())
    {
      if (stageIndex <= 0)
        threshold = data.weight;
      else if (stageIndex >= stage_max_get())
        threshold = data.stages.back().threshold;
      else
        threshold = data.stages[stageIndex - 1].threshold;
    }

    return system == measurement::IMPERIAL ? threshold * (double)measurement::KG_TO_LB : threshold;
  }

  float Character::stage_threshold_next_get(measurement::System system) const
  {
    return stage_threshold_get(stage + 1, system);
  }

  float Character::stage_progress_get()
  {
    auto currentStage = stage_get();
    if (currentStage >= stage_max_get()) return 1.0;

    auto currentThreshold = stage_threshold_get(currentStage);
    auto nextThreshold = stage_threshold_get(currentStage + 1);
    if (nextThreshold <= currentThreshold) return 1.0;

    return (weight - currentThreshold) / (nextThreshold - currentThreshold);
  }

  float Character::digestion_rate_get() { return digestionRate * 60.0; }

  float Character::max_capacity() const { return capacity * data.capacityMaxMultiplier; }
  bool Character::is_over_capacity() const { return calories > capacity; }
  bool Character::is_max_capacity() const { return calories >= max_capacity(); }
  float Character::capacity_percent_get() const { return calories / max_capacity(); }

  std::string Character::animation_append_id_get() const
  {
    if (stage <= 0) return {};
    auto stageIndex = stage - 1;
    if (stageIndex < 0 || stageIndex >= (int)data.stages.size()) return {};
    return data.stages[stageIndex].animationAppendID;
  }

  std::string Character::animation_name_convert(const std::string& name)
  {
    auto appendID = animation_append_id_get();
    return appendID.empty() ? name : name + appendID;
  }
  void Character::play_convert(const std::string& animation, Mode playMode, float startAtTime,
                               float speedMultiplierValue)
  {
    play(animation_name_convert(animation), playMode, startAtTime, speedMultiplierValue);
  }

  void Character::expand_areas_apply()
  {
    auto stageProgress = stage_progress_get();
    auto capacityProgress = isDigesting ? (double)calories / max_capacity() * (double)digestionTimer / data.digestionTimerMax
                                        : calories / max_capacity();

    for (int i = 0; i < (int)data.expandAreas.size(); i++)
    {
      auto& expandArea = data.expandAreas[i];
      auto& overrideLayer = overrides[expandAreaOverrideLayerIDs[i]];
      auto& overrideNull = overrides[expandAreaOverrideNullIDs[i]];

      auto stageScaleAdd = ((double)expandArea.scaleAdd * stageProgress) * 0.5;
      auto capacityScaleAdd = ((double)expandArea.scaleAdd * capacityProgress) * 0.5;

      auto scaleAdd =
          glm::clamp(glm::vec2(), glm::vec2(stageScaleAdd + capacityScaleAdd), glm::vec2(expandArea.scaleAdd));
      overrideLayer.frame.scale = scaleAdd;
      overrideNull.frame.scale = scaleAdd;
    }
  }

  void Character::update()
  {
    isJustStageUp = false;
    isJustStageFinal = false;
    isJustDigested = false;
  }

  void Character::tick()
  {
    if (state == Actor::STOPPED)
    {
      if (isStageUp)
      {
        if (stage >= (int)data.stages.size())
          isJustStageFinal = true;
        else
          isJustStageUp = true;

        isStageUp = false;
      }

      if (nextQueuedPlay.empty() && !isTalking) queue_idle_animation();
    }

    Actor::tick();

    if (isDigesting)
    {
      digestionTimer--;

      if (digestionTimer <= 0)
      {
        auto increment = calories * data.caloriesToKilogram;
        auto nextWeight = glm::clamp(weight + increment, data.weight, data.weightMax);

        if (is_over_capacity())
        {
          auto capacityMaxCalorieDifference = (calories - capacity);
          auto overCapacityPercent = capacityMaxCalorieDifference / (max_capacity() - capacity);
          auto capacityIncrement =
              (double)((overCapacityPercent * data.capacityIfOverStuffedOnDigestBonus) * capacityMaxCalorieDifference);
          capacity = glm::clamp(capacity + capacityIncrement, (double)data.capacityMin, (double)data.capacityMax);
        }

        totalCaloriesConsumed += calories;
        calories = 0;

        if (auto nextStage = stage_from_weight_get((double)nextWeight); nextStage > stage_from_weight_get(weight))
        {
          queuedPlay = QueuedPlay{};
          nextQueuedPlay = QueuedPlay{};
          currentQueuedPlay = QueuedPlay{};
          queue_play({.animation = data.animations.stageUp, .isInterruptible = false});
          stage = nextStage;
          isStageUp = true;
        }
        else
          isJustDigested = true;

        weight = (double)nextWeight;

        isDigesting = false;
        digestionTimer = data.digestionTimerMax;
        digestionProgress = 0;
      }
    }
    else
    {
      if (calories > 0) digestionProgress += digestionRate;
      if (digestionProgress >= DIGESTION_MAX)
      {
        isDigesting = true;
        digestionTimer = data.digestionTimerMax;
        data.sounds.digest.play();
      }
    }

    if (math::random_percent_roll(
            math::to_percent(data.gurgleChance * (capacity_percent_get() * data.gurgleCapacityMultiplier))))
      data.sounds.gurgle.play();

    stage = stage_get();
    expand_areas_apply();

    auto& talkOverride = overrides[talkOverrideID];

    if (isTalking)
    {
      auto talk_reset = [&]()
      {
        isTalking = false;
        talkTimer = 0.0;
        talkOverride.frame = FrameOptional();
      };

      auto& id = data.talkOverride.layerSource;
      auto& layerAnimations = animation_get()->layerAnimations;

      if (layerAnimations.contains(id) && animationTalkDurations.at(animationIndex) > -1)
      {
        auto& layerAnimation = layerAnimations.at(data.talkOverride.layerSource);

        if (!layerAnimation.frames.empty())
        {
          auto frame = frame_generate(layerAnimation, talkTimer, Anm2::LAYER, id);

          talkOverride.frame.crop = frame.crop;
          talkOverride.frame.size = frame.size;
          talkOverride.frame.pivot = frame.pivot;

          talkTimer += 1.0;

          if (talkTimer > animationTalkDurations.at(animationIndex)) talkTimer = 0.0;
        }
        else
          talk_reset();
      }
      else
        talk_reset();
    }
    else
      talkOverride.frame = {};

    auto& blinkOverride = overrides[blinkOverrideID];

    if (auto blinkDuration = animationBlinkDurations[animationIndex]; blinkDuration != 1)
    {
      if (math::random_percent_roll(data.blinkChance)) isBlinking = true;

      if (isBlinking)
      {
        auto blink_reset = [&]()
        {
          isBlinking = false;
          blinkTimer = 0.0;
          blinkOverride.frame = FrameOptional();
        };

        auto& id = data.blinkOverride.layerSource;
        auto& layerAnimations = animation_get()->layerAnimations;

        if (layerAnimations.contains(id))
        {
          auto& layerAnimation = layerAnimations.at(data.blinkOverride.layerSource);

          if (!layerAnimation.frames.empty())
          {
            auto frame = frame_generate(layerAnimation, blinkTimer, Anm2::LAYER, id);

            blinkOverride.frame.crop = frame.crop;
            blinkOverride.frame.size = frame.size;
            blinkOverride.frame.pivot = frame.pivot;

            blinkTimer += 1.0;

            if (blinkTimer >= blinkDuration) blink_reset();
          }
          else
            blink_reset();
        }
        else
          blink_reset();
      }
    }
  }

  void Character::queue_play(QueuedPlay play)
  {
    if (isStageUp) return;
    queuedPlay = play;
    queuedPlay.animation = animation_name_convert(queuedPlay.animation);
  }

  void Character::queue_idle_animation()
  {
    if (isStageUp) return;
    if (data.animations.idle.empty()) return;
    queue_play(
        {is_over_capacity() && !data.animations.idleFull.empty() ? data.animations.idleFull : data.animations.idle});
  }

  void Character::spritesheet_set(SpritesheetType type)
  {
    switch (type)
    {
      case NORMAL:
        spritesheets.at(data.alternateSpritesheet.id).texture =
            data.anm2.spritesheets.at(data.alternateSpritesheet.id).texture;
        break;
      case ALTERNATE:
        spritesheets.at(data.alternateSpritesheet.id).texture = data.alternateSpritesheet.texture;
        break;
      default:
        break;
    }

    spritesheetType = type;
  }
}
