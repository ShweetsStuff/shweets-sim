#pragma once

#include <glm/ext/vector_float4.hpp>

namespace game::util::color
{
  constexpr auto WHITE = glm::vec4(1.0f);
  constexpr auto GRAY = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
  constexpr auto BLACK = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}