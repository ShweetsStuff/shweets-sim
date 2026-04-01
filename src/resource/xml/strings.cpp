#include "strings.hpp"

#include "../../log.hpp"
#include "util.hpp"

#include <format>

using namespace tinyxml2;

namespace game::resource::xml
{
  namespace
  {
    std::string definition_element_name_get(const Strings::Definition& definition)
    {
      std::string name = definition.attribute;
      if (name.rfind("Text", 0) == 0) name.replace(0, 4, "String");
      return name;
    }
  }

  Strings::Strings()
  {
    for (int i = 0; i < Count; i++)
      values[i] = definitions[i].fallback;
  }

  Strings::Strings(const util::physfs::Path& path)
  {
    for (int i = 0; i < Count; i++)
      values[i] = definitions[i].fallback;

    XMLDocument document;
    if (document_load(path, document) != XML_SUCCESS) return;

    auto root = document.RootElement();
    if (!root) return;

    for (int i = 0; i < Count; i++)
      if (auto element = root->FirstChildElement(definition_element_name_get(definitions[i]).c_str()))
        query_string_attribute(element, "Text", &values[i]);

    isValid = true;
    logger.info(std::format("Initialized strings: {}", path.c_str()));
  }

  const std::string& Strings::get(Type type) const { return values[type]; }
}
