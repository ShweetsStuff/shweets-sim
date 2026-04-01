#include "preferences.hpp"

#include <SDL3/SDL_filesystem.h>

namespace game::util::preferences
{
  std::filesystem::path path()
  {
#ifdef __EMSCRIPTEN__
    static constexpr auto filePath = "/shweets-sim";
    std::filesystem::create_directories(filePath);
    return filePath;
#else
    auto sdlPath = SDL_GetPrefPath(nullptr, "shweets-sim");
    if (!sdlPath) return {};
    auto filePath = std::filesystem::path(sdlPath);
    std::filesystem::create_directories(filePath);
    SDL_free(sdlPath);
    return filePath;
#endif
  }
}