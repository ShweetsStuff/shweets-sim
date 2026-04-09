#include "dungeon.hpp"

#include "../../log.hpp"
#include "util.hpp"

#include <format>

using namespace tinyxml2;

namespace game::resource::xml
{
  Dungeon::Dungeon(const util::physfs::Path& path)
  {
    XMLDocument document;

    if (document_load(path, document) != XML_SUCCESS)
    {
      logger.error(std::format("Unable to initialize dungeon schema: {} ({})", path.c_str(), document.ErrorStr()));
      return;
    }

    if (auto root = document.RootElement())
    {
      if (std::string_view(root->Name()) != "Dungeon")
      {
        logger.error(std::format("Dungeon schema root element is not Dungeon: {}", path.c_str()));
        return;
      }

      query_string_attribute(root, "Title", &title);
      query_string_attribute(root, "Description", &description);
    }

    isValid = true;
    logger.info(std::format("Initialized dungeon schema: {}", path.c_str()));
  }

  bool Dungeon::is_valid() const { return isValid; };
}
