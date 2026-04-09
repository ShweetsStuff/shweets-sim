#include "dungeon.hpp"

#include "../../../../resource/font.hpp"
#include "../../../../resource/xml/strings.hpp"
#include "../../../../util/imgui/widget.hpp"
#include "../../../../util/math.hpp"

#include <format>
#include <imgui.h>

using namespace game::util::imgui;
using namespace game::resource::xml;

namespace game::state::play::menu::arcade
{
  int Dungeon::tile_value_get(const Tile& tile) const { return (int)tile.value; }
  bool Dungeon::tile_value_counts_toward_sum(const Tile& tile) const
  {
    return (tile.value >= Tile::VALUE_0 && tile.value <= Tile::VALUE_13) || tile.value == Tile::MINE;
  }
  bool Dungeon::tile_is_scroll(const Tile& tile) const { return tile.value == Tile::SCROLL; }
  const char* Dungeon::tile_flag_text_get(const Tile& tile) const
  {
    switch (tile.flagValue)
    {
      case Tile::FLAG_NONE:
        return nullptr;
      case Tile::FLAG_MINE:
        return "M";
      case Tile::FLAG_1:
        return "1";
      case Tile::FLAG_2:
        return "2";
      case Tile::FLAG_3:
        return "3";
      case Tile::FLAG_4:
        return "4";
      case Tile::FLAG_5:
        return "5";
      case Tile::FLAG_6:
        return "6";
      case Tile::FLAG_7:
        return "7";
      case Tile::FLAG_8:
        return "8";
      case Tile::FLAG_9:
        return "9";
      case Tile::FLAG_10:
        return "10";
      case Tile::FLAG_11:
        return "11";
      case Tile::FLAG_12:
        return "12";
      case Tile::FLAG_13:
        return "13";
      case Tile::FLAG_QUESTION:
        return "?";
    }

    return nullptr;
  }

  int Dungeon::surrounding_value_sum_get(int row, int column) const
  {
    auto sum = 0;
    for (int rowOffset = -1; rowOffset <= 1; rowOffset++)
      for (int columnOffset = -1; columnOffset <= 1; columnOffset++)
      {
        if (rowOffset == 0 && columnOffset == 0) continue;

        auto neighborRow = row + rowOffset;
        auto neighborColumn = column + columnOffset;
        if (neighborRow < 0 || neighborRow >= GRID_ROWS || neighborColumn < 0 || neighborColumn >= GRID_COLUMNS)
          continue;

        auto& neighbor = tiles[neighborRow * GRID_COLUMNS + neighborColumn];
        if (!tile_value_counts_toward_sum(neighbor)) continue;

        sum += tile_value_get(neighbor);
      }

    return sum;
  }

  void Dungeon::reveal_diamond(int row, int column, int radius)
  {
    for (int rowOffset = -radius; rowOffset <= radius; rowOffset++)
      for (int columnOffset = -radius; columnOffset <= radius; columnOffset++)
      {
        if (std::abs(rowOffset) + std::abs(columnOffset) > radius) continue;

        auto targetRow = row + rowOffset;
        auto targetColumn = column + columnOffset;
        if (targetRow < 0 || targetRow >= GRID_ROWS || targetColumn < 0 || targetColumn >= GRID_COLUMNS) continue;

        auto& tile = tiles[targetRow * GRID_COLUMNS + targetColumn];
        if (tile.state == Tile::HIDDEN)
        {
          tile.state = Tile::SHOWN;
          tile.flagValue = Tile::FLAG_NONE;
        }
      }
  }

  void Dungeon::reset(entity::Character&)
  {
    tiles.assign(GRID_ROWS * GRID_COLUMNS, Tile{});
    score = 0;
    for (auto& tile : tiles)
    {
      tile.value = game::util::math::random_percent_roll(5.0f) ? Tile::MINE
                                                               : (Tile::Value)(int)game::util::math::random_max(14.0f);
      tile.state = Tile::HIDDEN;
      tile.flagValue = Tile::FLAG_NONE;
    }

    if (!tiles.empty()) tiles[(int)game::util::math::random_max((float)tiles.size())].value = Tile::SCROLL;
  }

  void Dungeon::tick() {}

  bool Dungeon::update(entity::Character& character)
  {
    auto& strings = character.data.strings;
    constexpr float GRID_SPACING = 1.0f;
    auto& style = ImGui::GetStyle();

    if (tiles.size() != GRID_ROWS * GRID_COLUMNS) reset(character);

    auto contentRegionAvail = ImGui::GetContentRegionAvail();
    auto childSize =
        ImVec2(contentRegionAvail.x,
               std::max(0.0f, contentRegionAvail.y - ImGui::GetFrameHeightWithSpacing() - style.WindowPadding.y));

    if (ImGui::BeginChild("##DungeonGrid", childSize))
    {
      auto drawList = ImGui::GetWindowDrawList();
      auto childAvail = ImGui::GetContentRegionAvail();
      auto cellWidth = std::max(1.0f, (childAvail.x - GRID_SPACING * (GRID_COLUMNS - 1)) / (float)GRID_COLUMNS);
      auto cellHeight = std::max(1.0f, (childAvail.y - GRID_SPACING * (GRID_ROWS - 1)) / (float)GRID_ROWS);
      auto cellSize = std::floor(std::min(cellWidth, cellHeight));
      auto gridWidth = cellSize * (float)GRID_COLUMNS + GRID_SPACING * (GRID_COLUMNS - 1);
      auto gridHeight = cellSize * (float)GRID_ROWS + GRID_SPACING * (GRID_ROWS - 1);
      auto cursor = ImGui::GetCursorPos();
      auto offsetX = std::max(0.0f, (childAvail.x - gridWidth) * 0.5f);
      auto offsetY = std::max(0.0f, (childAvail.y - gridHeight) * 0.5f);

      ImGui::SetCursorPos(ImVec2(cursor.x + offsetX, cursor.y + offsetY));
      ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(GRID_SPACING, GRID_SPACING));

      for (int row = 0; row < GRID_ROWS; row++)
      {
        for (int column = 0; column < GRID_COLUMNS; column++)
        {
          auto tileID = row * GRID_COLUMNS + column;
          auto& tile = tiles[tileID];
          auto tileValue = tile_value_get(tile);

          ImGui::PushID(tileID);
          if (tile.state != Tile::HIDDEN)
          {
            auto buttonColor = style.Colors[ImGuiCol_WindowBg];
            ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColor);
          }
          auto isLeftPressed = WIDGET_FX(ImGui::Button("##DungeonCell", ImVec2(cellSize, cellSize)));
          auto isPopupOpen = ImGui::BeginPopupContextItem("##DungeonFlagMenu");
          if (isPopupOpen)
          {
            if (ImGui::Button("M", ImVec2(36.0f, 0.0f)))
            {
              tile.flagValue = Tile::FLAG_MINE;
              ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            for (int flagValue = Tile::FLAG_1; flagValue <= Tile::FLAG_13; flagValue++)
            {
              auto flagText = std::format("{}", flagValue);
              if (ImGui::Button(flagText.c_str(), ImVec2(36.0f, 0.0f)))
              {
                tile.flagValue = (Tile::FlagValue)flagValue;
                ImGui::CloseCurrentPopup();
              }
              if (flagValue % 4 != 0 && flagValue != Tile::FLAG_13) ImGui::SameLine();
            }
            if (ImGui::Button("?", ImVec2(36.0f, 0.0f)))
            {
              tile.flagValue = Tile::FLAG_QUESTION;
              ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear"))
            {
              tile.flagValue = Tile::FLAG_NONE;
              ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
          }
          auto rectMin = ImGui::GetItemRectMin();
          auto rectMax = ImGui::GetItemRectMax();
          if (tile.state != Tile::HIDDEN) ImGui::PopStyleColor(3);
          ImGui::PopID();

          if (isLeftPressed)
          {
            switch (tile.state)
            {
              case Tile::HIDDEN:
                tile.state = Tile::SHOWN;
                tile.flagValue = Tile::FLAG_NONE;
                if (tile_is_scroll(tile))
                {
                  reveal_diamond(row, column, 2);
                  tile.value = Tile::VALUE_0;
                  tile.state = Tile::SHOWN;
                }
                break;
              case Tile::SHOWN:
                tile.flagValue = Tile::FLAG_NONE;
                if (tile_is_scroll(tile))
                {
                  reveal_diamond(row, column, 2);
                  tile.value = Tile::VALUE_0;
                  tile.state = Tile::SHOWN;
                }
                else if (tileValue > 0)
                {
                  tile.state = Tile::CORPSE;
                  score += tileValue;
                }
                break;
              case Tile::CORPSE:
                tile.value = Tile::VALUE_0;
                tile.state = Tile::SHOWN;
                tile.flagValue = Tile::FLAG_NONE;
                break;
            }
          }

          std::string tileText{};
          auto textColor = IM_COL32(255, 255, 255, 255);
          if (tile_is_scroll(tile))
          {
            tileText = "!";
            if (tile.state == Tile::CORPSE)
              textColor = IM_COL32(255, 230, 64, 255);
            else
              textColor = IM_COL32(255, 255, 255, 255);
          }
          else if (tile.state == Tile::HIDDEN)
          {
            if (auto flagText = tile_flag_text_get(tile))
            {
              tileText = flagText;
              textColor = IM_COL32(64, 128, 255, 255);
            }
          }
          else
            switch (tile.state)
            {
              case Tile::HIDDEN:
                break;
              case Tile::SHOWN:
                if (tileValue == 0)
                {
                  auto surroundingSum = surrounding_value_sum_get(row, column);
                  if (surroundingSum > 0) tileText = std::format("{}", surroundingSum);
                }
                else
                {
                  tileText = std::format("{}", tileValue);
                  textColor = IM_COL32(255, 64, 64, 255);
                }
                break;
              case Tile::CORPSE:
                if (tileValue == 0)
                {
                  auto surroundingSum = surrounding_value_sum_get(row, column);
                  if (surroundingSum > 0) tileText = std::format("{}", surroundingSum);
                }
                else
                {
                  tileText = std::format("{}", tileValue);
                  textColor = IM_COL32(255, 230, 64, 255);
                }
                break;
            }

          if (!tileText.empty())
          {
            auto textSize = ImGui::CalcTextSize(tileText.c_str());
            auto textPosition = ImVec2(rectMin.x + (rectMax.x - rectMin.x - textSize.x) * 0.5f,
                                       rectMin.y + (rectMax.y - rectMin.y - textSize.y) * 0.5f);
            drawList->AddText(textPosition, textColor, tileText.c_str());
          }

          if (column + 1 < GRID_COLUMNS) ImGui::SameLine(0.0f, GRID_SPACING);
        }
      }

      ImGui::PopStyleVar(2);
    }
    ImGui::EndChild();
    ImGui::Text(strings.get(Strings::ArcadeScoreFormat).c_str(), score);

    return WIDGET_FX(ImGui::Button(strings.get(Strings::ArcadeBackButton).c_str()));
  }
}
