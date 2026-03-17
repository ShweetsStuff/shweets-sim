#pragma once

#include "../../util/measurement.hpp"
#include <filesystem>

#include <glm/glm.hpp>

namespace game::resource::xml
{
  class Settings
  {
  public:
    static constexpr auto VOLUME_MIN = 0;
    static constexpr auto VOLUME_MAX = 100;

    enum Mode
    {
      LOADER,
      IMGUI
    };

    util::measurement::System measurementSystem{util::measurement::METRIC};
    int volume{50};
    bool isUseCharacterColor{true};

    glm::vec3 color{0.120f, 0.515f, 0.115f};
    glm::ivec2 windowSize{1600, 900};
    glm::vec2 windowPosition{};

    bool isValid{};

    Settings() = default;
    Settings(const std::filesystem::path&);
    void serialize(const std::filesystem::path&);
    bool is_valid() const;
  };
}
