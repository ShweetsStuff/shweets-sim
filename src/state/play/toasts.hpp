#pragma once

#include <string>
#include <vector>

namespace game::state::play
{
  class Toasts
  {
  public:
    struct Item
    {
      static constexpr auto LIFETIME = 30;

      std::string message{};
      int lifetime{};
    };

    std::vector<Item> items{};

    void update();
    void tick();
    void push(const std::string&);
  };
};