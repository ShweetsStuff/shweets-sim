#include "area.hpp"

#include "../../log.hpp"
#include <format>

#include "util.hpp"

using namespace tinyxml2;
using namespace game::util;

namespace game::resource::xml
{
  Area::Area(const physfs::Path& path)
  {
    XMLDocument document;

    if (document_load(path, document) != XML_SUCCESS)
    {
      logger.error(std::format("Unable to initialize area schema: {} ({})", path.c_str(), document.ErrorStr()));
      return;
    }

    auto archive = path.directory_get();

    if (auto root = document.RootElement())
    {
      std::string textureRootPath{};
      query_string_attribute(root, "TextureRootPath", &textureRootPath);

      for (auto child = root->FirstChildElement("Area"); child; child = child->NextSiblingElement("Area"))
      {
        Entry area{};

        query_texture(child, "Texture", archive, textureRootPath, area.texture);
        child->QueryFloatAttribute("Gravity", &area.gravity);
        child->QueryFloatAttribute("Friction", &area.friction);

        areas.emplace_back(std::move(area));
      }
    }

    if (areas.empty()) areas.emplace_back(Entry());

    isValid = true;

    logger.info(std::format("Initialized area schema: {}", path.c_str()));
  }

  bool Area::is_valid() const { return isValid; };
}
