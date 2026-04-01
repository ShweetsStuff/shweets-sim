#include "actor.hpp"

#include "../util/map.hpp"
#include "../util/math.hpp"
#include "../util/unordered_map.hpp"
#include "../util/vector.hpp"

#include <cstddef>
#include <glm/glm.hpp>

#include "../log.hpp"

#include <imgui.h>

using namespace glm;
using namespace game::util;
using namespace game::resource::xml;

namespace game::entity
{
  Actor::Override::Override(int _id, Anm2::Type _type, Actor::Override::Mode _mode, FrameOptional _frame,
                            std::optional<float> _time, Actor::Override::Function _function, float _cycles)
      : id(_id), type(_type), mode(_mode), frame(_frame), time(_time), function(_function), cycles(_cycles)
  {
    frameBase = _frame;
    timeStart = _time;
  }

  Actor::Actor(const Actor&) = default;
  Actor::Actor(Actor&&) noexcept = default;
  Actor& Actor::operator=(const Actor&) = default;
  Actor& Actor::operator=(Actor&&) noexcept = default;

  Actor::Actor(Anm2 _anm2, vec2 _position, Mode playMode, float startAtTime, int startAnimationIndex)
      : Anm2(_anm2), position(_position)
  {
    this->mode = playMode;
    this->startTime = startAtTime;
    if (startAnimationIndex != -1)
      play(startAnimationIndex, playMode, startAtTime);
    else
      play_default_animation(playMode, startAtTime);
  }

  Anm2::Animation* Actor::animation_get(int index)
  {
    if (index == -1) index = animationIndex;
    if (animationMapReverse.contains(index)) return &animations[index];
    return nullptr;
  }

  Anm2::Animation* Actor::animation_get(const std::string& name)
  {
    if (animationMap.contains(name)) return &animations[animationMap[name]];
    return nullptr;
  }

  bool Actor::is_playing(const std::string& name)
  {
    if (name.empty())
      return state == PLAYING;
    else
      return state == PLAYING && animationMap[name] == animationIndex;
  }

  int Actor::animation_index_get(const std::string& name)
  {
    if (animationMap.contains(name)) return animationMap[name];
    return -1;
  }

  Anm2::Item* Actor::item_get(Anm2::Type type, int id, int checkAnimationIndex)
  {
    if (checkAnimationIndex == -1) checkAnimationIndex = this->animationIndex;
    if (auto animation = animation_get(checkAnimationIndex))
    {
      switch (type)
      {
        case Anm2::ROOT:
          return &animation->rootAnimation;
          break;
        case Anm2::LAYER:
          return unordered_map::find(animation->layerAnimations, id);
        case Anm2::NULL_:
          return map::find(animation->nullAnimations, id);
          break;
        case Anm2::TRIGGER:
          return &animation->triggers;
        default:
          return nullptr;
      }
    }

    return nullptr;
  }

  int Actor::item_length(Anm2::Item* item)
  {
    if (!item) return -1;

    int duration{};
    for (auto& frame : item->frames)
      duration += frame.duration;
    return duration;
  }

  Anm2::Frame Actor::frame_generate(Anm2::Item& item, float frameTime, Anm2::Type type, int id)
  {
    Anm2::Frame frame{};
    frame.isVisible = false;

    if (item.frames.empty()) return frame;

    frameTime = frameTime < 0.0f ? 0.0f : frameTime;

    Anm2::Frame* frameNext = nullptr;
    Anm2::Frame frameNextCopy{};
    int durationCurrent = 0;
    int durationNext = 0;

    for (int i = 0; i < (int)item.frames.size(); i++)
    {
      Anm2::Frame& checkFrame = item.frames[i];

      frame = checkFrame;

      durationNext += frame.duration;

      if (frameTime >= durationCurrent && frameTime < durationNext)
      {
        if (i + 1 < (int)item.frames.size())
        {
          frameNext = &item.frames[i + 1];
          frameNextCopy = *frameNext;
        }
        else
          frameNext = nullptr;
        break;
      }

      durationCurrent += frame.duration;
    }

    auto override_handle = [&](Anm2::Frame& overrideFrame)
    {
      for (auto& override : overrides)
      {
        if (override.type != type) continue;
        if (override.id != id) continue;

        auto& source = override.frame;

        switch (override.mode)
        {
          case Override::SET:
            if (source.position.has_value()) overrideFrame.position = *source.position;
            if (source.pivot.has_value()) overrideFrame.pivot = *source.pivot;
            if (source.size.has_value()) overrideFrame.size = *source.size;
            if (source.scale.has_value()) overrideFrame.scale = *source.scale;
            if (source.crop.has_value()) overrideFrame.crop = *source.crop;
            if (source.rotation.has_value()) overrideFrame.rotation = *source.rotation;
            if (source.tint.has_value()) overrideFrame.tint = *source.tint;
            if (source.colorOffset.has_value()) overrideFrame.colorOffset = *source.colorOffset;
            if (source.isInterpolated.has_value()) overrideFrame.isInterpolated = *source.isInterpolated;
            if (source.isVisible.has_value()) overrideFrame.isVisible = *source.isVisible;
            break;
          case Override::ADD:
            if (source.scale.has_value()) overrideFrame.scale += *source.scale;
            break;
          default:
            break;
        }
      }
    };

    override_handle(frame);
    if (frameNext) override_handle(frameNextCopy);

    if (frame.isInterpolated && frameNext && frame.duration > 1)
    {
      auto interpolation = (frameTime - durationCurrent) / (durationNext - durationCurrent);

      frame.rotation = glm::mix(frame.rotation, frameNextCopy.rotation, interpolation);
      frame.position = glm::mix(frame.position, frameNextCopy.position, interpolation);
      frame.scale = glm::mix(frame.scale, frameNextCopy.scale, interpolation);
      frame.colorOffset = glm::mix(frame.colorOffset, frameNextCopy.colorOffset, interpolation);
      frame.tint = glm::mix(frame.tint, frameNextCopy.tint, interpolation);
    }

    return frame;
  }

  void Actor::play(int index, Mode playMode, float startAtTime, float speedMultiplierValue)
  {
    if (!vector::in_bounds(animations, index)) return;
    if (playMode != PLAY_FORCE && index == animationIndex) return;

    this->playedEventID = -1;
    this->playedTriggers.clear();

    this->speedMultiplier = speedMultiplierValue;
    this->animationIndex = index;
    this->time = startAtTime;
    if (playMode == PLAY) state = PLAYING;
  }

  void Actor::queue_play(QueuedPlay newQueuedPlay) { queuedPlay = newQueuedPlay; }
  void Actor::queue_default_animation() { queue_play({defaultAnimation}); }

  void Actor::play(const std::string& name, Mode playMode, float startAtTime, float speedMultiplierValue)
  {
    if (animationMap.contains(name))
      play(animationMap.at(name), playMode, startAtTime, speedMultiplierValue);
    else
      logger.error(std::string("Animation \"" + name + "\" does not exist! Unable to play!"));
  }

  void Actor::play_default_animation(Mode playMode, float startAtTime, float speedMultiplierValue)
  {
    play(defaultAnimationID, playMode, startAtTime, speedMultiplierValue);
  }

  void Actor::tick()
  {
    if (state == Actor::STOPPED)
    {
      if (!nextQueuedPlay.empty())
      {
        queuedPlay = nextQueuedPlay;
        queuedPlay.isPlayAfterAnimation = false;
        nextQueuedPlay = QueuedPlay{};
      }
      currentQueuedPlay = QueuedPlay{};
    }

    if (auto animation = animation_get(); animation && animation->isLoop) currentQueuedPlay = QueuedPlay{};

    if (!queuedPlay.empty())
    {
      if (!animationMap.contains(queuedPlay.animation))
      {
        logger.error(std::string("Animation \"" + queuedPlay.animation + "\" does not exist! Unable to play!"));
        if (!defaultAnimation.empty()) queue_default_animation();
      }
      else
      {
        auto& index = animationMap.at(queuedPlay.animation);
        if (queuedPlay.isPlayAfterAnimation)
          nextQueuedPlay = queuedPlay;
        else if ((state == STOPPED || index != animationIndex) && currentQueuedPlay.isInterruptible)
        {
          play(queuedPlay.animation, queuedPlay.mode, queuedPlay.time, queuedPlay.speedMultiplier);
          currentQueuedPlay = queuedPlay;
        }
        queuedPlay = QueuedPlay{};
      }
    }

    auto animation = animation_get();
    if (!animation || animation->frameNum == 1 || mode == SET || state == STOPPED) return;

    playedEventID = -1;

    for (auto& trigger : animation->triggers.frames)
    {
      if (!playedTriggers.contains(trigger.atFrame) && time >= trigger.atFrame)
      {
        auto id = trigger.soundIDs[(int)math::random_max((float)trigger.soundIDs.size())];
        if (auto sound = map::find(sounds, id)) sound->audio.play();
        playedTriggers.insert((int)trigger.atFrame);
        playedEventID = trigger.eventID;
      }
    }

    auto increment = (fps / TICK_RATE) * speedMultiplier;
    time += increment;

    if (time >= animation->frameNum)
    {
      if (animation->isLoop)
        time = 0.0f;
      else
        state = STOPPED;

      playedTriggers.clear();
    }

    for (int i = 0; i < (int)overrides.size();)
    {
      auto& override_ = overrides[i];

      if (override_.function) override_.function(override_);

      if (override_.time.has_value())
      {
        *override_.time -= 1.0f;
        if (*override_.time <= 0.0f)
        {
          overrides.erase(overrides.begin() + i);
          continue;
        }
      }
      i++;
    }
  }

  glm::vec4 Actor::null_frame_rect(int nullID)
  {
    constexpr ivec2 CORNERS[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

    if (nullID == -1) return glm::vec4(NAN);
    auto item = item_get(Anm2::NULL_, nullID);
    if (!item) return glm::vec4(NAN);

    auto animation = animation_get();
    if (!animation) return glm::vec4(NAN);

    auto root = frame_generate(animation->rootAnimation, time, Anm2::ROOT);

    auto frame = frame_generate(*item, time, Anm2::NULL_, nullID);
    if (!frame.isVisible) return glm::vec4(NAN);

    auto rootModel =
        math::quad_model_no_size_get(root.position + position, root.pivot, math::to_unit(root.scale), root.rotation);
    auto frameModel = math::quad_model_get(frame.scale, frame.position, frame.scale * 0.5f, vec2(1.0f), frame.rotation);
    auto model = rootModel * frameModel;

    float minX = std::numeric_limits<float>::infinity();
    float minY = std::numeric_limits<float>::infinity();
    float maxX = -std::numeric_limits<float>::infinity();
    float maxY = -std::numeric_limits<float>::infinity();

    for (auto& corner : CORNERS)
    {
      vec4 world = model * vec4(corner, 0.0f, 1.0f);
      minX = std::min(minX, world.x);
      minY = std::min(minY, world.y);
      maxX = std::max(maxX, world.x);
      maxY = std::max(maxY, world.y);
    }

    return glm::vec4(minX, minY, maxX - minX, maxY - minY);
  }

  void Actor::render(resource::Shader& textureShader, resource::Shader& rectShader, Canvas& canvas)
  {
    auto animation = animation_get();
    if (!animation) return;

    auto root = frame_generate(animation->rootAnimation, time, Anm2::ROOT);

    auto rootModel =
        math::quad_model_no_size_get(root.position + position, root.pivot, math::to_unit(root.scale), root.rotation);

    for (auto& i : animation->layerOrder)
    {
      auto& layerAnimation = animation->layerAnimations[i];
      if (!layerAnimation.isVisible) continue;

      auto layer = map::find(layers, i);
      if (!layer) continue;

      auto spritesheet = map::find(spritesheets, layer->spritesheetID);
      if (!spritesheet) continue;

      auto frame = frame_generate(layerAnimation, time, Anm2::LAYER, i);
      if (!frame.isVisible) continue;

      auto model =
          math::quad_model_get(frame.size, frame.position, frame.pivot, math::to_unit(frame.scale), frame.rotation);
      model = rootModel * model;

      auto& texture = spritesheet->texture;
      if (!texture.is_valid()) return;

      auto tint = frame.tint * root.tint;
      auto colorOffset = frame.colorOffset + root.colorOffset;

      auto inset = vec2(0);
      auto uvMin = (frame.crop + inset) / vec2(texture.size);
      auto uvMax = (frame.crop + frame.size - inset) / vec2(texture.size);
      auto uvVertices = math::uv_vertices_get(uvMin, uvMax);

      canvas.texture_render(textureShader, texture.id, model, tint, colorOffset, uvVertices.data());
    }

    if (isShowNulls)
    {
      for (int i = 0; i < (int)animation->nullAnimations.size(); i++)
      {
        auto& nullAnimation = animation->nullAnimations[i];
        if (!nullAnimation.isVisible) continue;

        auto frame = frame_generate(nullAnimation, time, Anm2::NULL_, i);
        if (!frame.isVisible) continue;

        auto model = math::quad_model_get(frame.scale, frame.position, frame.scale * 0.5f, vec2(1.0f), frame.rotation);
        model = rootModel * model;

        canvas.rect_render(rectShader, model);
      }
    }
  }

  vec4 Actor::rect()
  {
    constexpr ivec2 CORNERS[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

    auto animation = animation_get();

    float minX = std::numeric_limits<float>::infinity();
    float minY = std::numeric_limits<float>::infinity();
    float maxX = -std::numeric_limits<float>::infinity();
    float maxY = -std::numeric_limits<float>::infinity();
    bool any = false;

    if (!animation) return vec4(-NAN);

    for (float t = 0.0f; t < (float)animation->frameNum; t += 1.0f)
    {
      mat4 transform(1.0f);

      auto root = frame_generate(animation->rootAnimation, t, Anm2::ROOT);
      transform *=
          math::quad_model_no_size_get(root.position + position, root.pivot, math::to_unit(root.scale), root.rotation);

      for (auto& [id, layerAnimation] : animation->layerAnimations)
      {
        if (!layerAnimation.isVisible) continue;

        auto frame = frame_generate(layerAnimation, t, Anm2::LAYER, id);

        if (frame.size == vec2() || !frame.isVisible) continue;

        auto layerTransform = transform * math::quad_model_get(frame.size, frame.position, frame.pivot,
                                                               math::to_unit(frame.scale), frame.rotation);
        for (auto& corner : CORNERS)
        {
          vec4 world = layerTransform * vec4(corner, 0.0f, 1.0f);
          minX = std::min(minX, world.x);
          minY = std::min(minY, world.y);
          maxX = std::max(maxX, world.x);
          maxY = std::max(maxY, world.y);
          any = true;
        }
      }
    }

    if (!any) return vec4(-NAN);
    return {minX, minY, maxX - minX, maxY - minY};
  }

  bool Actor::is_animation_finished()
  {
    if (auto animation = animation_get())
    {
      if (animation->isLoop) return true;
      if (time > animation->frameNum) return true;
    }
    return false;
  }

  void Actor::consume_played_event() { playedEventID = -1; }
};
