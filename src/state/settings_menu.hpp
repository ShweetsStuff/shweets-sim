#pragma once

#include "../resource/xml/strings.hpp"
#include "../resources.hpp"

namespace game::state
{
  class SettingsMenu
  {
  public:
    enum Mode
    {
      SELECT,
      PLAY
    };

    bool isGoToSelect{};
    bool isSave{};
    bool isJustColorSet{};

    void update(Resources&, Mode = SELECT, const resource::xml::Strings* = nullptr);
  };
}
