#pragma once

#include <unordered_set>

#include "../render/canvas.hpp"
#include "../resource/xml/anm2.hpp"

namespace game::entity
{
  class Actor : public resource::xml::Anm2
  {
  public:
    static constexpr auto TICK_RATE = 30.0f;

    enum Mode
    {
      PLAY,
      PLAY_FORCE,
      SET,
    };

    enum State
    {
      STOPPED,
      PLAYING
    };

    class Override
    {
    private:
    public:
      enum Mode
      {
        SET,
        ADD
      };

      using Function = void (*)(Override&);

      int id{-1};
      Anm2::Type type{Anm2::NONE};
      Mode mode{SET};
      FrameOptional frame{};
      std::optional<float> time{};
      Function function{nullptr};

      FrameOptional frameBase{};
      std::optional<float> timeStart{};

      float cycles{};

      Override() = default;
      Override(int, Anm2::Type, Mode, FrameOptional = {}, std::optional<float> = std::nullopt, Function = nullptr,
               float = 0);
    };

    struct QueuedPlay
    {
      std::string animation{};
      float time{};
      float speedMultiplier{1.0f};
      Mode mode{PLAY};
      bool isInterruptible{true};
      bool isPlayAfterAnimation{false};

      inline bool empty() { return animation.empty(); };
    };

    State state{STOPPED};
    Mode mode{PLAY};

    glm::vec2 position{};
    float time{};
    bool isShowNulls{};
    bool isVisible{true};
    int animationIndex{-1};
    int playedEventID{-1};
    float startTime{};
    float speedMultiplier{};

    QueuedPlay queuedPlay{};
    QueuedPlay currentQueuedPlay{};
    QueuedPlay nextQueuedPlay{};

    std::unordered_set<int> playedTriggers{};
    std::vector<Override> overrides{};

    Actor() = default;
    Actor(const Actor&);
    Actor(Actor&&) noexcept;
    Actor& operator=(const Actor&);
    Actor& operator=(Actor&&) noexcept;
    Actor(resource::xml::Anm2, glm::vec2 position = {}, Mode = PLAY, float time = 0.0f, int animationIndex = -1);
    bool is_playing(const std::string& name = {});
    glm::vec4 null_frame_rect(int = -1);
    glm::vec4 rect();
    int animation_index_get(const std::string&);
    int item_length(resource::xml::Anm2::Item*);
    resource::xml::Anm2::Animation* animation_get(const std::string&);
    resource::xml::Anm2::Animation* animation_get(int = -1);
    resource::xml::Anm2::Frame frame_generate(resource::xml::Anm2::Item&, float, resource::xml::Anm2::Type,
                                              int id = -1);
    resource::xml::Anm2::Item* item_get(resource::xml::Anm2::Type, int = -1, int = -1);
    void consume_played_event();
    void play(const std::string& animation, Mode = PLAY, float time = 0.0f, float speedMultiplier = 1.0f);
    void play(int index, Mode = PLAY, float time = 0.0f, float speedMultiplier = 1.0f);
    void play_default_animation(Mode = PLAY, float = 0.0f, float = 1.0f);
    void queue_default_animation();
    void queue_play(QueuedPlay);
    bool is_animation_finished();
    void render(resource::Shader& textureShader, resource::Shader& rectShader, Canvas&);
    void tick();
  };
}
