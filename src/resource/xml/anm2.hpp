#pragma once

#include <optional>
#include <tinyxml2/tinyxml2.h>

#include <filesystem>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "../audio.hpp"
#include "../texture.hpp"

#include "../../util/physfs.hpp"

namespace game::resource::xml
{
  class Anm2
  {
  public:
    enum Type
    {
      NONE,
      ROOT,
      LAYER,
      NULL_,
      TRIGGER
    };

    enum Flag
    {
      NO_SPRITESHEETS = (1 << 0),
      NO_SOUNDS = (1 << 1),
      DEFAULT_ANIMATION_ONLY = (1 << 2)
    };

    using Flags = int;

    struct Spritesheet
    {
      struct Region
      {
        enum Origin
        {
          TOP_LEFT,
          CENTER,
          CUSTOM
        };

        std::string name{};
        glm::vec2 crop{};
        glm::vec2 pivot{};
        glm::vec2 size{};
        Origin origin{CUSTOM};
      };

      std::string path{};
      resource::Texture texture{};

      std::map<int, Region> regions{};
      std::vector<int> regionOrder{};
    };

    struct Sound
    {
      std::string path{};
      resource::Audio audio{};
    };

    struct Layer
    {
      std::string name{"New Layer"};
      int spritesheetID{-1};
    };

    struct Null
    {
      std::string name{"New Null"};
      bool isShowRect{};
    };

    struct Event
    {
      std::string name{"New Event"};
    };

    struct Frame
    {
      glm::vec2 crop{};
      glm::vec2 position{};
      glm::vec2 pivot{};
      glm::vec2 size{};
      glm::vec2 scale{100, 100};
      float rotation{};
      int duration{};
      glm::vec4 tint{1.0f, 1.0f, 1.0f, 1.0f};
      glm::vec3 colorOffset{};
      bool isInterpolated{};
      int eventID{-1};
      int regionID{-1};
      std::vector<int> soundIDs{};
      int atFrame{-1};

      bool isVisible{true};
    };

    struct FrameOptional
    {
      struct Vec2
      {
        std::optional<float> x{};
        std::optional<float> y{};

        Vec2() = default;
        Vec2(const glm::vec2& value) : x(value.x), y(value.y) {}
        Vec2& operator=(const glm::vec2& value)
        {
          x = value.x;
          y = value.y;
          return *this;
        }
      };

      struct Vec3
      {
        std::optional<float> x{};
        std::optional<float> y{};
        std::optional<float> z{};

        Vec3() = default;
        Vec3(const glm::vec3& value) : x(value.x), y(value.y), z(value.z) {}
        Vec3& operator=(const glm::vec3& value)
        {
          x = value.x;
          y = value.y;
          z = value.z;
          return *this;
        }
      };

      struct Vec4
      {
        std::optional<float> x{};
        std::optional<float> y{};
        std::optional<float> z{};
        std::optional<float> w{};

        Vec4() = default;
        Vec4(const glm::vec4& value) : x(value.x), y(value.y), z(value.z), w(value.w) {}
        Vec4& operator=(const glm::vec4& value)
        {
          x = value.x;
          y = value.y;
          z = value.z;
          w = value.w;
          return *this;
        }
      };

      Vec2 crop{};
      Vec2 position{};
      Vec2 pivot{};
      Vec2 size{};
      Vec2 scale{};
      std::optional<float> rotation{};
      Vec4 tint{};
      Vec3 colorOffset{};
      std::optional<bool> isInterpolated{};
      std::optional<bool> isVisible{};
    };

    struct Item
    {
      std::vector<Frame> frames{};
      bool isVisible{};
    };

    struct Animation
    {
      std::string name{"New Animation"};
      int frameNum{};
      bool isLoop{};

      Item rootAnimation{};
      std::unordered_map<int, Item> layerAnimations{};
      std::vector<int> layerOrder{};
      std::map<int, Item> nullAnimations{};
      Item triggers{};
    };

    int fps{30};

    std::map<int, Spritesheet> spritesheets{};
    std::map<int, Layer> layers{};
    std::map<int, Null> nulls{};
    std::map<int, Event> events{};
    std::map<int, Sound> sounds{};

    std::unordered_map<std::string, int> layerMap{};
    std::unordered_map<std::string, int> nullMap{};
    std::unordered_map<std::string, int> eventMap{};

    std::string defaultAnimation{};
    int defaultAnimationID{-1};
    std::vector<Animation> animations{};
    std::unordered_map<std::string, int> animationMap{};
    std::unordered_map<int, std::string> animationMapReverse{};
    std::string path{};
    bool isValid{};
    Flags flags{};

    Anm2() = default;
    Anm2(const Anm2&);
    Anm2(Anm2&&);
    Anm2& operator=(const Anm2&);
    Anm2& operator=(Anm2&&);
    Anm2(const std::filesystem::path&, Flags = 0);
    Anm2(const util::physfs::Path&, Flags = 0);

    bool is_valid() const;

  private:
    void init(tinyxml2::XMLDocument& document, Flags anm2Flags, const util::physfs::Path& archive = {});
  };
}
