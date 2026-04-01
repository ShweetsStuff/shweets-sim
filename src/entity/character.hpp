#pragma once

#include "../resource/xml/character.hpp"
#include "../util/measurement.hpp"

#include "actor.hpp"

namespace game::entity
{
  class Character : public Actor
  {
  public:
    static constexpr auto DIGESTION_MAX = 100.0f;

    enum SpritesheetType
    {
      NORMAL,
      ALTERNATE
    };

    resource::xml::Character data;

    float weight{};
    int stage{0};
    float calories{};
    float capacity{};

    float digestionProgress{};
    float digestionRate{0.05f};
    int digestionTimer{};
    bool isDigesting{};
    bool isJustDigested{};

    float eatSpeed{1.0f};

    float totalCaloriesConsumed{};
    int totalFoodItemsEaten{};

    int talkOverrideID{};
    float talkTimer{};
    bool isTalking{};
    std::vector<int> animationTalkDurations{};

    int blinkOverrideID{};
    float blinkTimer{};
    bool isBlinking{};
    std::vector<int> animationBlinkDurations{};

    bool isStageUp{};
    bool isStageUpDuring{};
    bool isJustStageUp{};
    bool isJustStageFinal{};

    SpritesheetType spritesheetType{};

    std::map<int, Override> interactAreaOverrides{};

    std::unordered_map<int, int> expandAreaOverrideLayerIDs{};
    std::unordered_map<int, int> expandAreaOverrideNullIDs{};

    Character() = default;
    Character(const Character&);
    Character(Character&&) noexcept;
    Character& operator=(const Character&);
    Character& operator=(Character&&) noexcept;
    Character(resource::xml::Character&, glm::ivec2);

    float weight_get(util::measurement::System = util::measurement::METRIC);
    float digestion_rate_get();
    float capacity_percent_get() const;
    float max_capacity() const;
    bool is_over_capacity() const;
    bool is_max_capacity() const;

    int stage_get() const;
    int stage_from_weight_get(float weight) const;
    int stage_max_get() const;
    float stage_progress_get();
    float stage_threshold_get(int stage = -1, util::measurement::System = util::measurement::METRIC) const;
    float stage_threshold_next_get(util::measurement::System = util::measurement::METRIC) const;

    void expand_areas_apply();
    void spritesheet_set(SpritesheetType);
    void update();
    void tick();
    void play_convert(const std::string&, Mode = PLAY, float time = 0.0f, float speedMultiplier = 1.0f);
    void queue_idle_animation();
    void queue_play(QueuedPlay);

    std::string animation_append_id_get() const;
    std::string animation_name_convert(const std::string& name);
  };
}
