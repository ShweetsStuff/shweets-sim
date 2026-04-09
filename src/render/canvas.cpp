#include "canvas.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <utility>

#include "../util/imgui.hpp"
#include "../util/math.hpp"

using namespace glm;
using namespace game::resource;
using namespace game::util;

namespace game
{
  GLuint Canvas::textureVAO = 0;
  GLuint Canvas::textureVBO = 0;
  GLuint Canvas::textureEBO = 0;
  GLuint Canvas::rectVAO = 0;
  GLuint Canvas::rectVBO = 0;
  bool Canvas::isStaticInit = false;

  Canvas::Canvas(ivec2 size, Flags flags)
  {
    this->size = size;
    this->flags = flags;

    if ((flags & DEFAULT) != 0)
    {
      fbo = 0;
      rbo = 0;
      texture = 0;
    }
    else
    {
      glGenFramebuffers(1, &fbo);
      glGenRenderbuffers(1, &rbo);
      glGenTextures(1, &texture);

      glBindFramebuffer(GL_FRAMEBUFFER, fbo);

      glBindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

      glBindRenderbuffer(GL_RENDERBUFFER, rbo);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    if (!isStaticInit)
    {
      glGenVertexArrays(1, &textureVAO);
      glGenBuffers(1, &textureVBO);
      glGenBuffers(1, &textureEBO);

      glBindVertexArray(textureVAO);

      glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 4, nullptr, GL_DYNAMIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textureEBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(TEXTURE_INDICES), TEXTURE_INDICES, GL_DYNAMIC_DRAW);

      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

      glBindVertexArray(0);

      glGenVertexArrays(1, &rectVAO);
      glGenBuffers(1, &rectVBO);

      glBindVertexArray(rectVAO);

      glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(RECT_VERTICES), RECT_VERTICES, GL_STATIC_DRAW);

      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

      glBindVertexArray(0);

      isStaticInit = true;
    }
  }

  Canvas::Canvas(const Canvas& other) : Canvas(other.size, other.flags)
  {
    pan = other.pan;
    shakeOffset = other.shakeOffset;
    zoom = other.zoom;
    shakeTimer = other.shakeTimer;
    shakeTimerMax = other.shakeTimerMax;

    if ((flags & DEFAULT) == 0 && (other.flags & DEFAULT) == 0)
    {
      glBindFramebuffer(GL_READ_FRAMEBUFFER, other.fbo);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
      glBlitFramebuffer(0, 0, other.size.x, other.size.y, 0, 0, size.x, size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
  }

  Canvas::Canvas(Canvas&& other) noexcept
  {
    size = other.size;
    pan = other.pan;
    shakeOffset = other.shakeOffset;
    zoom = other.zoom;
    shakeTimer = other.shakeTimer;
    shakeTimerMax = other.shakeTimerMax;
    flags = other.flags;
    fbo = other.fbo;
    rbo = other.rbo;
    texture = other.texture;

    other.size = {};
    other.pan = {};
    other.shakeOffset = {};
    other.zoom = 100.0f;
    other.shakeTimer = 0;
    other.shakeTimerMax = 0;
    other.flags = FLIP;
    other.fbo = 0;
    other.rbo = 0;
    other.texture = 0;
  }

  Canvas::~Canvas()
  {
    if ((flags & DEFAULT) == 0)
    {
      if (fbo) glDeleteFramebuffers(1, &fbo);
      if (rbo) glDeleteRenderbuffers(1, &rbo);
      if (texture) glDeleteTextures(1, &texture);
    }
  }

  Canvas& Canvas::operator=(const Canvas& other)
  {
    if (this == &other) return *this;
    Canvas tmp(other);
    *this = std::move(tmp);
    return *this;
  }

  Canvas& Canvas::operator=(Canvas&& other) noexcept
  {
    if (this == &other) return *this;

    if ((flags & DEFAULT) == 0)
    {
      if (fbo) glDeleteFramebuffers(1, &fbo);
      if (rbo) glDeleteRenderbuffers(1, &rbo);
      if (texture) glDeleteTextures(1, &texture);
    }

    size = other.size;
    pan = other.pan;
    shakeOffset = other.shakeOffset;
    zoom = other.zoom;
    shakeTimer = other.shakeTimer;
    shakeTimerMax = other.shakeTimerMax;
    flags = other.flags;
    fbo = other.fbo;
    rbo = other.rbo;
    texture = other.texture;

    other.size = {};
    other.pan = {};
    other.shakeOffset = {};
    other.zoom = 100.0f;
    other.shakeTimer = 0;
    other.shakeTimerMax = 0;
    other.flags = FLIP;
    other.fbo = 0;
    other.rbo = 0;
    other.texture = 0;

    return *this;
  }

  mat4 Canvas::view_get() const
  {
    auto view = mat4{1.0f};
    auto zoomFactor = math::to_unit(zoom);
    auto panFactor = pan * zoomFactor;

    view = glm::translate(view, vec3(-panFactor, 0.0f));
    view = glm::scale(view, vec3(zoomFactor, zoomFactor, 1.0f));

    return view;
  }
  mat4 Canvas::projection_get() const
  {
    if ((flags & FLIP) != 0)
    {
      return glm::ortho(0.0f, (float)size.x, 0.0f, (float)size.y, -1.0f, 1.0f);
    }
    return glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, -1.0f, 1.0f);
  }

  void Canvas::texture_render(Shader& shader, GLuint textureID, mat4 model, vec4 tint, vec3 colorOffset,
                              const float* vertices) const
  {
    glUseProgram(shader.id);

    glUniform1i(glGetUniformLocation(shader.id, shader::UNIFORM_TEXTURE), 0);
    glUniform3fv(glGetUniformLocation(shader.id, shader::UNIFORM_COLOR_OFFSET), 1, value_ptr(colorOffset));
    glUniform4fv(glGetUniformLocation(shader.id, shader::UNIFORM_TINT), 1, value_ptr(tint));

    glUniformMatrix4fv(glGetUniformLocation(shader.id, shader::UNIFORM_MODEL), 1, GL_FALSE, value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader.id, shader::UNIFORM_VIEW), 1, GL_FALSE, value_ptr(view_get()));
    glUniformMatrix4fv(glGetUniformLocation(shader.id, shader::UNIFORM_PROJECTION), 1, GL_FALSE,
                       value_ptr(projection_get()));

    glBindVertexArray(textureVAO);

    glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TEXTURE_VERTICES), vertices, GL_DYNAMIC_DRAW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
  }

  void Canvas::rect_render(Shader& shader, mat4& model, vec4 color) const
  {
    glUseProgram(shader.id);

    glUniform4fv(glGetUniformLocation(shader.id, shader::UNIFORM_COLOR), 1, value_ptr(color));

    glUniformMatrix4fv(glGetUniformLocation(shader.id, shader::UNIFORM_MODEL), 1, GL_FALSE, value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader.id, shader::UNIFORM_VIEW), 1, GL_FALSE, value_ptr(view_get()));
    glUniformMatrix4fv(glGetUniformLocation(shader.id, shader::UNIFORM_PROJECTION), 1, GL_FALSE,
                       value_ptr(projection_get()));

    glBindVertexArray(rectVAO);

    glDrawArrays(GL_LINE_LOOP, 0, 4);

    glBindVertexArray(0);
    glUseProgram(0);
  }

  void Canvas::texture_render(Shader& shader, const Canvas& source, mat4 model, vec4 tint, vec3 colorOffset) const
  {
    auto shakenModel = glm::translate(model, glm::vec3(source.shakeOffset, 0.0f));
    texture_render(shader, source.texture, shakenModel, tint, colorOffset);
  }

  void Canvas::render(Shader& shader, mat4& model, vec4 tint, vec3 colorOffset) const
  {
    auto shakenModel = glm::translate(model, glm::vec3(shakeOffset, 0.0f));
    texture_render(shader, texture, shakenModel, tint, colorOffset);
  }

  void Canvas::shake(float magnitude, int timeTicks)
  {
    shakeTimerMax = std::max(1, timeTicks);
    shakeTimer = shakeTimerMax;
    shakeOffset = {math::random_in_range(-magnitude, magnitude), math::random_in_range(-magnitude, magnitude)};
  }

  void Canvas::tick()
  {
    static constexpr auto SHAKE_LERP_FACTOR = 0.35f;
    static constexpr auto SHAKE_DECAY = 0.85f;
    static constexpr auto SHAKE_EPSILON = 0.1f;

    if (shakeTimer > 0)
    {
      shakeTimer--;

      auto magnitude = glm::length(shakeOffset) * SHAKE_DECAY;
      auto timerFactor = (float)shakeTimer / (float)std::max(1, shakeTimerMax);
      auto target =
          glm::vec2(math::random_in_range(-magnitude, magnitude), math::random_in_range(-magnitude, magnitude)) *
          timerFactor;
      shakeOffset = glm::mix(shakeOffset, target, SHAKE_LERP_FACTOR);
    }
    else
    {
      shakeOffset = glm::mix(shakeOffset, glm::vec2{}, SHAKE_LERP_FACTOR);
      if (glm::length(shakeOffset) < SHAKE_EPSILON)
      {
        shakeOffset = {};
        shakeTimerMax = 0;
      }
    }
  }

  void Canvas::bind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  }

  void Canvas::size_set(ivec2 newSize)
  {
    if ((flags & DEFAULT) == 0 && (newSize.x != this->size.x || newSize.y != this->size.y))
    {
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newSize.x, newSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      glBindTexture(GL_TEXTURE_2D, 0);

      glBindRenderbuffer(GL_RENDERBUFFER, rbo);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, newSize.x, newSize.y);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    this->size = newSize;
    glViewport(0, 0, newSize.x, newSize.y);
  }

  void Canvas::clear(vec4 color)
  {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  void Canvas::unbind() const
  {
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  bool Canvas::is_valid() const { return (flags & DEFAULT) != 0 || fbo != 0; };

  glm::vec2 Canvas::screen_position_convert(glm::vec2 position) const
  {
    auto viewport = ImGui::GetMainViewport();
    auto viewportPos = imgui::to_vec2(viewport->Pos);
    auto localPosition = position - viewportPos;
    auto zoomFactor = math::to_unit(zoom);
    return pan + (localPosition / zoomFactor);
  }
}
