#include "cursor.hpp"

#include "../../log.hpp"

#include <format>

using namespace tinyxml2;
using namespace game::util;

namespace game::resource::xml
{
  Cursor::Cursor(const physfs::Path& path)
  {
    XMLDocument document;

    if (document_load(path, document) != XML_SUCCESS)
    {
      logger.error(std::format("Unable to initialize cursor schema: {} ({})", path.c_str(), document.ErrorStr()));
      return;
    }

    auto archive = path.directory_get();

    if (auto root = document.RootElement())
    {
      std::string textureRootPath{};
      query_string_attribute(root, "TextureRootPath", &textureRootPath);

      std::string soundRootPath{};
      query_string_attribute(root, "SoundRootPath", &soundRootPath);

      query_anm2(root, "Anm2", archive, textureRootPath, anm2);

      if (auto element = root->FirstChildElement("Animations"))
      {
        query_animation_entry_collection(element, "Idle", animations.idle);
        query_animation_entry_collection(element, "Hover", animations.hover);
        query_animation_entry_collection(element, "Grab", animations.grab);
        query_animation_entry_collection(element, "Pan", animations.pan);
        query_animation_entry_collection(element, "Zoom", animations.zoom);
        query_animation_entry_collection(element, "Return", animations.return_);
      }

      if (auto element = root->FirstChildElement("Sounds"))
      {
        query_sound_entry_collection(element, "Grab", archive, soundRootPath, sounds.grab);
        query_sound_entry_collection(element, "Release", archive, soundRootPath, sounds.release);
        query_sound_entry_collection(element, "Throw", archive, soundRootPath, sounds.throw_);
      }
    }

    isValid = true;

    logger.info(std::format("Initialized area schema: {}", path.c_str()));
  }

  bool Cursor::is_valid() const { return isValid; };
}
