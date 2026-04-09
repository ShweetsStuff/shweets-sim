#include "play.hpp"
#include "play/style.hpp"

#include <array>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include "../util/imgui.hpp"
#include "../util/imgui/style.hpp"
#include "../util/imgui/widget.hpp"
#include "../util/math.hpp"

using namespace game::resource;
using namespace game::resource::xml;
using namespace game::util;
using namespace game::state::play;
using namespace glm;

namespace game::state
{
  namespace
  {
    int durability_animation_index_get(const resource::xml::Item& schema, const resource::xml::Anm2& anm2, int durability,
                                       int durabilityMax)
    {
      if (durability >= durabilityMax) return -1;

      auto animationName = schema.animations.chew + std::to_string(std::max(0, durability));
      return anm2.animationMap.contains(animationName) ? anm2.animationMap.at(animationName) : -1;
    }
  }

  World::Focus Play::focus_get()
  {
    if (!isWindows) return World::CENTER;

    return menu.isOpen && tools.isOpen ? World::MENU_TOOLS
           : menu.isOpen               ? World::MENU
           : tools.isOpen              ? World::TOOLS
                                       : World::CENTER;
  }

  void Play::set(Resources& resources, int selectedCharacterIndex, enum Game game)
  {
    auto& data = resources.character_get(selectedCharacterIndex);
    auto& saveData = data.save;
    auto& itemSchema = data.itemSchema;
    auto& dialogue = data.dialogue;
    auto& menuSchema = data.menuSchema;
    this->characterIndex = selectedCharacterIndex;
    cheatCodeIndex = 0;
    cheatCodeStartTime = 0.0;

    character =
        entity::Character(data, vec2(World::BOUNDS.x + World::BOUNDS.z * 0.5f, World::BOUNDS.w - World::BOUNDS.y));
    character.digestionRate =
        glm::clamp(character.digestionRate, (float)data.digestionRateMin, (float)data.digestionRateMax);
    character.eatSpeed = glm::clamp(character.eatSpeed, (float)data.eatSpeedMin, (float)data.eatSpeedMax);
    character.capacity = glm::clamp(character.capacity, (float)data.capacityMin, (float)data.capacityMax);

    auto isAlternateSpritesheet =
        (game == NEW_GAME && math::random_percent_roll(data.alternateSpritesheet.chanceOnNewGame));

    if (isAlternateSpritesheet || saveData.isAlternateSpritesheet)
    {
      character.spritesheet_set(entity::Character::ALTERNATE);
      if (game == NEW_GAME) character.data.alternateSpritesheet.sound.play();
    }

    character.totalCaloriesConsumed = saveData.totalCaloriesConsumed;
    character.totalFoodItemsEaten = saveData.totalFoodItemsEaten;
    characterManager = CharacterManager{};

    cursor = entity::Cursor(character.data.cursorSchema.anm2);
    cursor.interactTypeID = character.data.interactTypeNames.empty() ? -1 : 0;

    menu.inventory = play::menu::Inventory{};
    for (auto& [id, quantity] : saveData.inventory)
    {
      if (quantity == 0) continue;
      menu.inventory.values[id] = quantity;
    }

    itemManager = ItemManager{};
    for (auto& item : saveData.items)
    {
      auto& anm2 = itemSchema.anm2s.at(item.id);
      auto& schemaItem = itemSchema.items.at(item.id);
      auto durabilityMax = schemaItem.durability.value_or(itemSchema.durability);
      auto animationIndex = durability_animation_index_get(itemSchema, anm2, item.durability, durabilityMax);
      auto& saveItem = itemSchema.anm2s.at(item.id);
      itemManager.items.emplace_back(saveItem, item.position, item.id, item.durability, animationIndex, item.velocity,
                                     item.rotation);
    }

    imgui::style::widget_set(menuSchema.rounding);
    imgui::widget::sounds_set(&menuSchema.sounds.hover, &menuSchema.sounds.select);
    play::style::color_set(resources, character);

    menu.arcade = play::menu::Arcade(character);
    menu.arcade.skillCheck.totalPlays = saveData.skillCheck.totalPlays;
    menu.arcade.skillCheck.highScore = saveData.skillCheck.highScore;
    menu.arcade.skillCheck.bestCombo = saveData.skillCheck.bestCombo;
    menu.arcade.skillCheck.gradeCounts = saveData.skillCheck.gradeCounts;
    menu.arcade.skillCheck.isHighScoreAchieved = saveData.skillCheck.highScore > 0 ? true : false;
    menu.arcade.orbit.highScore = saveData.orbit.highScore;

    text.entry = nullptr;
    text.isEnabled = false;

#if DEBUG
    menu.isCheats = true;
#else
    menu.isCheats = false;
#endif

    isPostgame = saveData.isPostgame;
    if (character.stage_get() >= character.stage_max_get()) isPostgame = true;
    if (isPostgame) menu.isCheats = true;

    if (game == NEW_GAME) isWindows = false;

    if (auto font = character.data.menuSchema.font.get()) ImGui::GetIO().FontDefault = font;

    character.queue_idle_animation();
    character.tick();
    worldCanvas.size_set(imgui::to_vec2(ImGui::GetMainViewport()->Size));
    world.set(character, worldCanvas, focus_get());

    if (game == NEW_GAME && dialogue.start.is_valid())
    {
      character.queue_play({.animation = dialogue.start.animation, .isInterruptible = false});
      character.tick();
      isStart = true;
      isStartBegin = false;
      isStartEnd = false;
    }

    if (isPostgame)
    {
      isEnd = true;
      isEndBegin = true;
      isEndEnd = true;
    }
    else
    {
      isEnd = false;
      isEndBegin = false;
      isEndEnd = false;
    }
  }

  void Play::exit(Resources& resources)
  {
    imgui::style::color_set(resources.settings.color);
    imgui::style::widget_set();
    imgui::widget::sounds_set(nullptr, nullptr);
    ImGui::GetIO().FontDefault = resources.font.get();
    save(resources);
  }

  void Play::tick(Resources&)
  {
    character.tick();
    cursor.tick();
    menu.tick();
    toasts.tick();
    text.tick(character);

    for (auto& item : itemManager.items)
      item.tick();
  }

  void Play::update(Resources& resources)
  {
    static constexpr std::array<ImGuiKey, 10> CHEAT_CODE = {
        ImGuiKey_UpArrow,    ImGuiKey_UpArrow,   ImGuiKey_DownArrow,  ImGuiKey_DownArrow, ImGuiKey_LeftArrow,
        ImGuiKey_RightArrow, ImGuiKey_LeftArrow, ImGuiKey_RightArrow, ImGuiKey_B,         ImGuiKey_A};
    static constexpr std::array<ImGuiKey, 6> CHEAT_INPUT_KEYS = {
        ImGuiKey_UpArrow, ImGuiKey_DownArrow, ImGuiKey_LeftArrow, ImGuiKey_RightArrow, ImGuiKey_B, ImGuiKey_A};
    static constexpr auto CHEAT_CODE_INPUT_TIME_SECONDS = 5.0;

    auto focus = focus_get();
    auto& dialogue = character.data.dialogue;
    cursor.isVisible = true;

    if (!menu.isCheats)
    {
      for (auto key : CHEAT_INPUT_KEYS)
      {
        if (!ImGui::IsKeyPressed(key, false)) continue;

        if (key == CHEAT_CODE[cheatCodeIndex])
        {
          cheatCodeIndex++;
          cheatCodeStartTime = ImGui::GetTime();
        }
        else if (key == CHEAT_CODE[0])
        {
          cheatCodeIndex = 1;
          cheatCodeStartTime = ImGui::GetTime();
        }
        else
        {
          cheatCodeIndex = 0;
          cheatCodeStartTime = 0.0;
        }

        if (cheatCodeIndex >= (int)CHEAT_CODE.size())
        {
          menu.isCheats = true;
          cheatCodeIndex = 0;
          cheatCodeStartTime = 0.0;
          toasts.push(character.data.strings.get(Strings::ToastCheatsUnlocked));
          character.data.menuSchema.sounds.cheatsActivated.play();
        }
      }

      if (cheatCodeIndex > 0 && (ImGui::GetTime() - cheatCodeStartTime > CHEAT_CODE_INPUT_TIME_SECONDS))
      {
        cheatCodeIndex = 0;
        cheatCodeStartTime = 0.0;
      }
    }

    if (isWindows)
    {
      menu.update(resources, itemManager, character, cursor, text, worldCanvas);
      tools.update(character, cursor, world, focus, worldCanvas);
      info.update(resources, character);
      toasts.update();
    }

    if (text.isEnabled) text.update(character);

    if (isStart)
    {
      if (!isStartBegin)
      {
        if (auto animation = character.animation_get())
        {
          if (animation->isLoop || character.state == entity::Actor::STOPPED)
          {
            text.set(dialogue.get(dialogue.start.id), character);
            isStartBegin = true;
          }
        }
      }
      else if (!isStartEnd)
      {
        if (text.entry->is_last())
        {
          isWindows = true;
          isStartEnd = true;
          isStart = false;
          world.character_focus(character, worldCanvas, focus_get());
        }
      }
    }

    if (character.isJustStageFinal && !isEnd && !isPostgame) isEnd = true;

    if (isEnd)
    {
      if (!isEndBegin)
      {
        if (character.is_animation_finished())
        {
          text.set(dialogue.get(dialogue.end.id), character);
          isEndBegin = true;
          isWindows = false;
          tools.isOpen = false;
          menu.isOpen = false;
          character.calories = 0;
          character.digestionProgress = 0;
          itemManager.items.clear();
          itemManager.heldItemIndex = -1;
          world.character_focus(character, worldCanvas, focus_get());
        }
      }
      else if (!isEndEnd)
      {
        if (text.entry->is_last())
        {
          menu.isOpen = true;
          isWindows = true;
          isEndEnd = true;
          isEnd = false;
          isPostgame = true;
          menu.isCheats = true;
          world.character_focus(character, worldCanvas, focus_get());
        }
      }
    }

    itemManager.update(character, cursor, areaManager, text, World::BOUNDS, worldCanvas);
    characterManager.update(character, cursor, text, worldCanvas);

    character.update();
    cursor.update();
    world.update(character, cursor, worldCanvas, focus);
    worldCanvas.tick();

    if (autosaveTime += ImGui::GetIO().DeltaTime; autosaveTime > AUTOSAVE_TIME || menu.settingsMenu.isSave)
    {
      save(resources);
      autosaveTime = 0;
      menu.settingsMenu.isSave = false;
    }
  }

  void Play::render(Resources& resources, Canvas& canvas)
  {
    auto& textureShader = resources.shaders[shader::TEXTURE];
    auto& rectShader = resources.shaders[shader::RECT];
    auto size = imgui::to_ivec2(ImGui::GetMainViewport()->Size);

    auto& bgTexture = character.data.areaSchema.areas.at(areaManager.get(character)).texture;

    auto windowModel = math::quad_model_get(vec2(size));
    auto worldModel = math::quad_model_get(bgTexture.size);
    worldCanvas.bind();
    worldCanvas.size_set(size);
    worldCanvas.clear();
    worldCanvas.texture_render(textureShader, bgTexture.id, worldModel);

    character.render(textureShader, rectShader, worldCanvas);

    for (auto& item : itemManager.items)
      item.render(textureShader, rectShader, worldCanvas);

    if (menu.debug.isBoundsDisplay)
    {
      auto boundsModel =
          math::quad_model_get(glm::vec2(World::BOUNDS.z, World::BOUNDS.w), glm::vec2(World::BOUNDS.x, World::BOUNDS.y),
                               glm::vec2(World::BOUNDS.x, World::BOUNDS.y) * 0.5f);
      worldCanvas.rect_render(rectShader, boundsModel);
    }
    worldCanvas.unbind();

    canvas.bind();
    canvas.texture_render(textureShader, worldCanvas, windowModel);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    cursor.render(textureShader, rectShader, canvas);
    canvas.unbind();
  }

  void Play::save(Resources& resources)
  {
    resource::xml::Save save;

    save.weight = character.weight;
    save.calories = character.calories;
    save.capacity = character.capacity;
    save.digestionRate = character.digestionRate;
    save.eatSpeed = character.eatSpeed;
    save.digestionProgress = character.digestionProgress;
    save.isDigesting = character.isDigesting;
    save.digestionTimer = character.digestionTimer;
    save.totalCaloriesConsumed = character.totalCaloriesConsumed;
    save.totalFoodItemsEaten = character.totalFoodItemsEaten;
    save.skillCheck.totalPlays = menu.arcade.skillCheck.totalPlays;
    save.skillCheck.highScore = menu.arcade.skillCheck.highScore;
    save.skillCheck.bestCombo = menu.arcade.skillCheck.bestCombo;
    save.skillCheck.gradeCounts = menu.arcade.skillCheck.gradeCounts;
    save.orbit.highScore = menu.arcade.orbit.highScore;
    save.isPostgame = isPostgame;
    save.isAlternateSpritesheet = character.spritesheetType == entity::Character::ALTERNATE;

    for (auto& [id, quantity] : menu.inventory.values)
    {
      if (quantity == 0) continue;
      save.inventory[id] = quantity;
    }

    for (auto& item : itemManager.items)
      save.items.emplace_back(item.schemaID, item.durability, item.position, item.velocity,
                              *item.overrides[item.rotationOverrideID].frame.rotation);

    save.isValid = true;

    resources.character_save_set(characterIndex, save);
    save.serialize(character.data.save_path_get());

    toasts.push(character.data.strings.get(Strings::ToastSaving));
  }
};
