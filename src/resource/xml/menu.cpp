#include "menu.hpp"

#include "../../log.hpp"

#include <format>

#include "util.hpp"

using namespace tinyxml2;
using namespace game::util;

namespace game::resource::xml
{
  Menu::Menu(const physfs::Path& path)
  {
    XMLDocument document;

    if (document_load(path, document) != XML_SUCCESS)
    {
      logger.error(std::format("Unable to initialize menu schema: {} ({})", path.c_str(), document.ErrorStr()));
      return;
    }

    auto archive = path.directory_get();

    if (auto root = document.RootElement())
    {
      std::string soundRootPath{};
      query_string_attribute(root, "SoundRootPath", &soundRootPath);

      std::string fontRootPath{};
      query_string_attribute(root, "FontRootPath", &fontRootPath);

      query_font(root, "Font", archive, fontRootPath, font);
      root->QueryFloatAttribute("Rounding", &rounding);

      if (auto element = root->FirstChildElement("Sounds"))
      {
        query_sound_entry_collection(element, "Open", archive, soundRootPath, sounds.open);
        query_sound_entry_collection(element, "Close", archive, soundRootPath, sounds.close);
        query_sound_entry_collection(element, "Hover", archive, soundRootPath, sounds.hover);
        query_sound_entry_collection(element, "Select", archive, soundRootPath, sounds.select);
        query_sound_entry_collection(element, "CheatsActivated", archive, soundRootPath, sounds.cheatsActivated);
      }
    }

    isValid = true;

    logger.info(std::format("Initialized menu schema: {}", path.c_str()));
  }

  bool Menu::is_valid() const { return isValid; };
}
