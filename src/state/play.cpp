#include "play.hpp"

#include <array>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include "../util/imgui.hpp"
#include "../util/imgui/style.hpp"
#include "../util/imgui/widget.hpp"
#include "../util/math.hpp"

using namespace game::resource;
using namespace game::util;
using namespace game::state::play;
using namespace glm;

namespace game::state
{
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
    character.digestionRate = glm::clamp(data.digestionRateMin, character.digestionRate, data.digestionRateMax);
    character.eatSpeed = glm::clamp(data.eatSpeedMin, character.eatSpeed, data.eatSpeedMax);
    character.capacity = glm::clamp(data.capacityMin, character.capacity, data.capacityMax);

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

    menu.inventory = Inventory{};
    for (auto& [id, quantity] : saveData.inventory)
    {
      if (quantity == 0) continue;
      menu.inventory.values[id] = quantity;
    }

    itemManager = ItemManager{};
    for (auto& item : saveData.items)
    {
      auto& anm2 = itemSchema.anm2s.at(item.id);
      auto chewAnimation = itemSchema.animations.chew + std::to_string(item.chewCount);
      auto animationIndex = item.chewCount > 0 ? anm2.animationMap[chewAnimation] : -1;
      auto& saveItem = itemSchema.anm2s.at(item.id);
      itemManager.items.emplace_back(saveItem, item.position, item.id, item.chewCount, animationIndex, item.velocity,
                                     item.rotation);
    }

    imgui::style::rounding_set(menuSchema.rounding);
    imgui::widget::sounds_set(&menuSchema.sounds.hover, &menuSchema.sounds.select);
    menu.color_set_check(resources, character);

    menu.skillCheck = SkillCheck(character);
    menu.skillCheck.totalPlays = saveData.totalPlays;
    menu.skillCheck.highScore = saveData.highScore;
    menu.skillCheck.bestCombo = saveData.bestCombo;
    menu.skillCheck.gradeCounts = saveData.gradeCounts;
    menu.skillCheck.isHighScoreAchieved = saveData.highScore > 0 ? true : false;
    menu.isChat = character.data.dialogue.help.is_valid() || character.data.dialogue.random.is_valid();

    text.entry = nullptr;
    text.isEnabled = false;

    menu.isCheats = false;
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
    imgui::style::rounding_set();
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
          toasts.push("Cheats unlocked!");
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
    canvas.texture_render(textureShader, worldCanvas.texture, windowModel);
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
    save.totalPlays = menu.skillCheck.totalPlays;
    save.highScore = menu.skillCheck.highScore;
    save.bestCombo = menu.skillCheck.bestCombo;
    save.gradeCounts = menu.skillCheck.gradeCounts;
    save.isPostgame = isPostgame;
    save.isAlternateSpritesheet = character.spritesheetType == entity::Character::ALTERNATE;

    for (auto& [id, quantity] : menu.inventory.values)
    {
      if (quantity == 0) continue;
      save.inventory[id] = quantity;
    }

    for (auto& item : itemManager.items)
      save.items.emplace_back(item.schemaID, item.chewCount, item.position, item.velocity,
                              *item.overrides[item.rotationOverrideID].frame.rotation);

    save.isValid = true;

    resources.character_save_set(characterIndex, save);
    save.serialize(character.data.save_path_get());

    toasts.push("Saving...");
  }
};
