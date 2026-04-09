#pragma once

#include "../../util/physfs.hpp"

#include <array>
#include <string>

namespace game::resource::xml
{
#define GAME_XML_STRING_LIST(X)                                                                                        \
  X(MenuTabInteract, "TextMenuTabInteract", "Interact")                                                                \
  X(MenuTabArcade, "TextMenuTabArcade", "Arcade")                                                                      \
  X(MenuTabInventory, "TextMenuTabInventory", "Inventory")                                                             \
  X(MenuTabSettings, "TextMenuTabSettings", "Settings")                                                                \
  X(MenuTabCheats, "TextMenuTabCheats", "Cheats")                                                                      \
  X(MenuOpenTooltip, "TextMenuOpenTooltip", "Open Main Menu")                                                          \
  X(MenuCloseTooltip, "TextMenuCloseTooltip", "Close Main Menu")                                                       \
  X(InteractChatButton, "TextInteractChatButton", "Let's chat!")                                                       \
  X(InteractHelpButton, "TextInteractHelpButton", "Help")                                                              \
  X(InteractFeelingButton, "TextInteractFeelingButton", "How are you feeling?")                                        \
  X(InteractWeightFormat, "TextInteractWeightFormat", "Weight: %0.2f %s (Stage: %i)")                                  \
  X(InteractCapacityFormat, "TextInteractCapacityFormat", "Capacity: %0.0f kcal (Max: %0.0f kcal)")                    \
  X(InteractDigestionRateFormat, "TextInteractDigestionRateFormat", "Digestion Rate: %0.2f%%/sec")                     \
  X(InteractEatingSpeedFormat, "TextInteractEatingSpeedFormat", "Eating Speed: %0.2fx")                                \
  X(InteractTotalCaloriesFormat, "TextInteractTotalCaloriesFormat", "Total Calories Consumed: %0.0f kcal")             \
  X(InteractTotalFoodItemsFormat, "TextInteractTotalFoodItemsFormat", "Total Food Items Eaten: %i")                    \
  X(SettingsMeasurementSystem, "TextSettingsMeasurementSystem", "Measurement System")                                  \
  X(SettingsMetric, "TextSettingsMetric", "Metric")                                                                    \
  X(SettingsMetricTooltip, "TextSettingsMetricTooltip", "Use kilograms (kg).")                                         \
  X(SettingsImperial, "TextSettingsImperial", "Imperial")                                                              \
  X(SettingsImperialTooltip, "TextSettingsImperialTooltip", "Use pounds (lbs).")                                       \
  X(SettingsSound, "TextSettingsSound", "Sound")                                                                       \
  X(SettingsVolume, "TextSettingsVolume", "Volume")                                                                    \
  X(SettingsVolumeTooltip, "TextSettingsVolumeTooltip", "Adjust master volume.")                                       \
  X(SettingsAppearance, "TextSettingsAppearance", "Appearance")                                                        \
  X(SettingsUseCharacterColor, "TextSettingsUseCharacterColor", "Use Character Color")                                 \
  X(SettingsUseCharacterColorTooltip, "TextSettingsUseCharacterColorTooltip",                                          \
    "When playing, the UI will use the character's preset UI color.")                                                  \
  X(SettingsColor, "TextSettingsColor", "Color")                                                                       \
  X(SettingsColorTooltip, "TextSettingsColorTooltip", "Change the UI color.")                                          \
  X(SettingsResetButton, "TextSettingsResetButton", "Reset to Default")                                                \
  X(SettingsSaveButton, "TextSettingsSaveButton", "Save")                                                              \
  X(SettingsSaveTooltip, "TextSettingsSaveTooltip", "Save the game.\n(Note: the game autosaves frequently.)")          \
  X(SettingsReturnToCharactersButton, "TextSettingsReturnToCharactersButton", "Return to Characters")                  \
  X(SettingsReturnToCharactersTooltip, "TextSettingsReturnToCharactersTooltip",                                        \
    "Go back to the character selection screen.\nProgress will be saved.")                                             \
  X(ToastCheatsUnlocked, "TextToastCheatsUnlocked", "Cheats unlocked!")                                                \
  X(ToastSaving, "TextToastSaving", "Saving...")                                                                       \
  X(ToolsHomeButton, "TextToolsHomeButton", "Home")                                                                    \
  X(ToolsHomeTooltip, "TextToolsHomeTooltip", "Reset camera view.\n(Shortcut: Home)")                                  \
  X(ToolsOpenTooltip, "TextToolsOpenTooltip", "Open Tools")                                                            \
  X(ToolsCloseTooltip, "TextToolsCloseTooltip", "Close Tools")                                                         \
  X(InventoryEmptyHint, "TextInventoryEmptyHint", "Check the \"Arcade\" tab to earn rewards!")                         \
  X(InventoryFlavorFormat, "TextInventoryFlavorFormat", "Flavor: %s")                                                  \
  X(InventoryCaloriesFormat, "TextInventoryCaloriesFormat", "%0.0f kcal")                                              \
  X(InventoryDurabilityFormat, "TextInventoryDurabilityFormat", "Durability: %i")                                      \
  X(InventoryCapacityBonusFormat, "TextInventoryCapacityBonusFormat", "Capacity Bonus: +%0.0f kcal")                   \
  X(InventoryDigestionRateBonusFormat, "TextInventoryDigestionRateBonusFormat",                                        \
    "Digestion Rate Bonus: +%0.2f%% / sec")                                                                            \
  X(InventoryDigestionRatePenaltyFormat, "TextInventoryDigestionRatePenaltyFormat",                                    \
    "Digestion Rate Penalty: %0.2f%% / sec")                                                                           \
  X(InventoryEatSpeedBonusFormat, "TextInventoryEatSpeedBonusFormat", "Eat Speed Bonus: +%0.2f%% / sec")               \
  X(InventoryEatSpeedPenaltyFormat, "TextInventoryEatSpeedPenaltyFormat", "Eat Speed Penalty: %0.2f%% / sec")          \
  X(InventoryUpgradePreviewFormat, "TextInventoryUpgradePreviewFormat", "Upgrade: %ix -> %s")                          \
  X(InventoryUnknown, "TextInventoryUnknown", "???")                                                                   \
  X(InventorySpawnButton, "TextInventorySpawnButton", "Spawn")                                                         \
  X(InventoryUpgradeButton, "TextInventoryUpgradeButton", "Upgrade")                                                   \
  X(InventoryUpgradeAllButton, "TextInventoryUpgradeAllButton", "Upgrade All")                                         \
  X(InventoryUpgradeNoPath, "TextInventoryUpgradeNoPath", "This item cannot be upgraded.")                             \
  X(InventoryUpgradeNeedsTemplate, "TextInventoryUpgradeNeedsTemplate", "Needs {}x to upgrade into {}!")               \
  X(InventoryUpgradeOneTemplate, "TextInventoryUpgradeOneTemplate", "Use {}x to upgrade into 1x {}.")                  \
  X(InventoryUpgradeAllTemplate, "TextInventoryUpgradeAllTemplate", "Use {}x to upgrade into {}x {}.")                 \
  X(ArcadeSkillCheckName, "TextArcadeSkillCheckName", "Skill Check")                                                   \
  X(ArcadeSkillCheckDescription, "TextArcadeSkillCheckDescription",                                                    \
    "Test your timing! Aim for specific zones for rewards.")                                                           \
  X(ArcadeDungeonName, "TextArcadeDungeonName", "Dungeon")                                                             \
  X(ArcadeDungeonDescription, "TextArcadeDungeonDescription",                                                          \
    "A placeholder dungeon adventure entry. Use this as a template for a future arcade game.")                         \
  X(ArcadeOrbitName, "TextArcadeOrbitName", "Orbit")                                                                   \
  X(ArcadeOrbitDescription, "TextArcadeOrbitDescription",                                                              \
    "Move colored objects orbiting around the cursor into similarly colored enemies!")                                 \
  X(ArcadeHowToPlay, "TextArcadeHowToPlay", "How to Play")                                                             \
  X(ArcadeSkillCheckHowToPlay, "TextArcadeSkillCheckHowToPlay",                                                        \
    "Press Space or click to stop the line inside the colored target zones.\nEach success builds score and combo, "    \
    "and high scores improve your reward chances, while increasing game speed and tightening the zones.\nMissing the " \
    "colored zones ends the run.")                                                                                     \
  X(ArcadeDungeonHowToPlay, "TextArcadeDungeonHowToPlay",                                                              \
    "This is currently a template page.\nUse it to prototype dungeon rules, rewards, room flow, and UI layout.")      \
  X(ArcadeOrbitHowToPlay, "TextArcadeOrbitHowToPlay",                                                                  \
    "Control an object with your cursor.\nThere will be colored, orbiting objects around it.\nUse the mouse buttons "  \
    "to move the objects around your orbit.\nEnemies will appear with the same color as the orbiting objects.\nMatch " \
    "the colors of the orbiting objects into the same colored enemies for score.\nOver time, more colors will be "     \
    "added nearby the cursor, and more enemies will appear.\nEnemies will also get faster with time.\nHow long can "   \
    "you survive?!")                                                                                                   \
  X(ArcadeDungeonTemplateTitle, "TextArcadeDungeonTemplateTitle", "Dungeon Template")                                  \
  X(ArcadeDungeonTemplateBody, "TextArcadeDungeonTemplateBody",                                                        \
    "This screen is a placeholder for the Dungeon arcade game.\nAdd room generation, encounters, rewards, and any "   \
    "character-specific hooks here.\nUse the Back button below to return to the arcade menu.")                         \
  X(ArcadeScoreFormat, "TextArcadeScoreFormat", "Score: %i pts")                                                       \
  X(ArcadeScoreComboFormat, "TextArcadeScoreComboFormat", "Score: %i pts (%ix)")                                       \
  X(ArcadePlayButton, "TextArcadePlayButton", "Play")                                                                  \
  X(ArcadeInfoButton, "TextArcadeInfoButton", "Info")                                                                  \
  X(ArcadeBackButton, "TextArcadeBackButton", "Back")                                                                  \
  X(ArcadeStats, "TextArcadeStats", "Stats")                                                                           \
  X(ArcadeBestScoreFormat, "TextArcadeBestScoreFormat", "Best: %i pts")                                                \
  X(ArcadeBestScoreComboFormat, "TextArcadeBestScoreComboFormat", "Best: %i pts (%ix)")                               \
  X(ArcadeTotalSkillChecksFormat, "TextArcadeTotalSkillChecksFormat", "Rounds Attempted: %i")                          \
  X(ArcadeAccuracyFormat, "TextArcadeAccuracyFormat", "Accuracy: %0.2f%%")                                             \
  X(InfoProgressMax, "TextInfoProgressMax", "MAX")                                                                     \
  X(InfoProgressToNextStage, "TextInfoProgressToNextStage", "To Next Stage")                                           \
  X(InfoStageProgressFormat, "TextInfoStageProgressFormat", "Stage: %i/%i (%0.1f%%)")                                  \
  X(InfoMaxedOut, "TextInfoMaxedOut", "Maxed out!")                                                                    \
  X(InfoStageStartFormat, "TextInfoStageStartFormat", "Start: %0.2f %s")                                               \
  X(InfoStageCurrentFormat, "TextInfoStageCurrentFormat", "Current: %0.2f %s")                                         \
  X(InfoStageNextFormat, "TextInfoStageNextFormat", "Next: %0.2f %s")                                                  \
  X(InfoDigestion, "TextInfoDigestion", "Digestion")                                                                   \
  X(InfoDigesting, "TextInfoDigesting", "Digesting...")                                                                \
  X(InfoDigestionInProgress, "TextInfoDigestionInProgress", "Digestion in progress...")                                \
  X(InfoGiveFoodToStartDigesting, "TextInfoGiveFoodToStartDigesting", "Give food to start digesting!")                 \
  X(InfoDigestionRateFormat, "TextInfoDigestionRateFormat", "Rate: %0.2f%% / sec")                                     \
  X(InfoEatingSpeedFormat, "TextInfoEatingSpeedFormat", "Eating Speed: %0.2fx")                                        \
  X(SkillCheckInstructions, "TextSkillCheckInstructions",                                                              \
    "Match the line to the colored areas with Space/click! Better performance, better rewards!")                       \
  X(ArcadeScoreLoss, "TextArcadeScoreLoss", "-1")                                                                      \
  X(ArcadeRewardToast, "TextArcadeRewardToast", "Fantastic score! Congratulations!")                                   \
  X(ArcadeHighScoreToast, "TextArcadeHighScoreToast", "High Score!")                                                   \
  X(ArcadeMenuBackButtonTooltip, "TextArcadeMenuBackButtonTooltip", "Progress will not be saved!")                     \
  X(SkillCheckGradeSuccessTemplate, "TextSkillCheckGradeSuccessTemplate", "{} (+{})")                                  \
  X(ArcadeMenuBackButton, "TextArcadeMenuBackButton", "Menu")                                                          \
  X(CheatsCalories, "TextCheatsCalories", "Calories")                                                                  \
  X(CheatsCapacity, "TextCheatsCapacity", "Capacity")                                                                  \
  X(CheatsWeight, "TextCheatsWeight", "Weight")                                                                        \
  X(CheatsWeightFormat, "TextCheatsWeightFormat", "%0.2f kg")                                                          \
  X(CheatsStage, "TextCheatsStage", "Stage")                                                                           \
  X(CheatsDigestionRate, "TextCheatsDigestionRate", "Digestion Rate")                                                  \
  X(CheatsDigestionRateFormat, "TextCheatsDigestionRateFormat", "%0.2f% / tick")                                       \
  X(CheatsEatSpeed, "TextCheatsEatSpeed", "Eat Speed")                                                                 \
  X(CheatsEatSpeedFormat, "TextCheatsEatSpeedFormat", "%0.2fx")                                                        \
  X(CheatsDigestButton, "TextCheatsDigestButton", "Digest")                                                            \
  X(CheatsInventory, "TextCheatsInventory", "Inventory")

  class Strings
  {
  public:
    enum Type
    {
#define X(type, attr, fallback) type,
      GAME_XML_STRING_LIST(X)
#undef X
          Count
    };

    struct Definition
    {
      const char* attribute;
      const char* fallback;
    };

    inline static constexpr std::array<Definition, Count> definitions{{
#define X(type, attr, fallback) {attr, fallback},
        GAME_XML_STRING_LIST(X)
#undef X
    }};

    std::array<std::string, Count> values{};

    bool isValid{};

    Strings();
    Strings(const util::physfs::Path&);

    const std::string& get(Type) const;
  };
}
