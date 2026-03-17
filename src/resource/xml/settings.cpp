#include "settings.hpp"
#include "util.hpp"

#include <tinyxml2/tinyxml2.h>

#include "../../log.hpp"

#include <format>

#ifdef __EMSCRIPTEN__
  #include "../../util/web_filesystem.hpp"
#endif

using namespace tinyxml2;
using namespace game::util;

namespace game::resource::xml
{
  Settings::Settings(const std::filesystem::path& path)
  {
    XMLDocument document;
    auto pathString = path.string();

    if (document.LoadFile(pathString.c_str()) != XML_SUCCESS)
    {
      logger.error(std::format("Could not initialize character save file: {} ({})", pathString, document.ErrorStr()));
      return;
    }

    if (auto root = document.RootElement())
    {
      std::string measurementSystemString{};
      query_string_attribute(root, "MeasurementSystem", &measurementSystemString);
      measurementSystem = measurementSystemString == "Imperial" ? measurement::IMPERIAL : measurement::METRIC;
      root->QueryIntAttribute("Volume", &volume);
      query_vec3(root, "ColorR", "ColorG", "ColorB", color);
      query_vec2(root, "WindowX", "WindowY", windowPosition);
      query_ivec2(root, "WindowW", "WindowH", windowSize);
      query_bool_attribute(root, "IsUseCharacterColor", &isUseCharacterColor);
    }

    logger.info(std::format("Initialized settings: {}", pathString));

    isValid = true;
  }

  bool Settings::is_valid() const { return isValid; }

  void Settings::serialize(const std::filesystem::path& path)
  {
    XMLDocument document;
    auto pathString = path.string();

    auto element = document.NewElement("Settings");

    element->SetAttribute("MeasurementSystem", measurementSystem == measurement::IMPERIAL ? "Imperial" : "Metric");
    element->SetAttribute("Volume", volume);
    set_vec3_attribute(element, "ColorR", "ColorG", "ColorB", color);
    set_vec2_attribute(element, "WindowX", "WindowY", windowPosition);
    set_ivec2_attribute(element, "WindowW", "WindowH", windowSize);
    set_bool_attribute(element, "IsUseCharacterColor", isUseCharacterColor);

    document.InsertFirstChild(element);

    if (document.SaveFile(pathString.c_str()) != XML_SUCCESS)
    {
      logger.info(std::format("Failed to initialize settings: {} ({})", pathString, document.ErrorStr()));
      return;
    }

    logger.info(std::format("Saved settings: {}", pathString));

#ifdef __EMSCRIPTEN__
    web_filesystem::flush_async();
#endif
  }
}
