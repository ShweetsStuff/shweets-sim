#pragma once

#ifdef __EMSCRIPTEN__
  #include <GLES3/gl3.h>
#else
  #include <glad/glad.h>
#endif

#include "../resource/shader.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

namespace game
{
  class Canvas
  {
    static constexpr float TEXTURE_VERTICES[] = {0, 0, 0.0f, 0.0f, 1, 0, 1.0f, 0.0f,
                                                 1, 1, 1.0f, 1.0f, 0, 1, 0.0f, 1.0f};

    static constexpr GLuint TEXTURE_INDICES[] = {0, 1, 2, 2, 3, 0};

    static constexpr float RECT_VERTICES[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};

    static GLuint textureVAO;
    static GLuint textureVBO;
    static GLuint textureEBO;
    static GLuint rectVAO;
    static GLuint rectVBO;
    static bool isStaticInit;

  public:
    static constexpr glm::vec4 CLEAR_COLOR = {0, 0, 0, 0};
    static constexpr float SHAKE_MAGNITUDE = 0.05f;
    static constexpr float SHAKE_TIME_TICKS = 60;

    enum Flag
    {
      DEFAULT = (1 << 0),
      FLIP = (1 << 1)
    };

    using Flags = int;

    GLuint fbo{};
    GLuint rbo{};
    GLuint texture{};

    glm::ivec2 size{};
    glm::vec2 pan{};
    glm::vec2 shakeOffset{};
    float zoom{100.0f};
    int shakeTimer{};
    int shakeTimerMax{};
    Flags flags{FLIP};

    Canvas() = default;
    Canvas(glm::ivec2, Flags = FLIP);
    Canvas(const Canvas&);
    Canvas(Canvas&&) noexcept;
    ~Canvas();
    Canvas& operator=(const Canvas&);
    Canvas& operator=(Canvas&&) noexcept;
    glm::mat4 transform_get() const;
    glm::mat4 view_get() const;
    glm::mat4 projection_get() const;
    void texture_render(resource::Shader&, GLuint, glm::mat4, glm::vec4 = glm::vec4(1.0f), glm::vec3 = {},
                        const float* = TEXTURE_VERTICES) const;
    void texture_render(resource::Shader&, const Canvas&, glm::mat4, glm::vec4 = glm::vec4(1.0f), glm::vec3 = {}) const;
    void rect_render(resource::Shader&, glm::mat4&, glm::vec4 = glm::vec4(0, 0, 1, 1)) const;
    void render(resource::Shader&, glm::mat4&, glm::vec4 = glm::vec4(1.0f), glm::vec3 = {}) const;
    void shake(float magnitude = SHAKE_MAGNITUDE, int timeTicks = SHAKE_TIME_TICKS);
    void tick();
    void bind();
    void size_set(glm::ivec2 size);
    void clear(glm::vec4 color = CLEAR_COLOR);
    void unbind() const;
    bool is_valid() const;
    glm::vec2 screen_position_convert(glm::vec2 position) const;
  };
}
