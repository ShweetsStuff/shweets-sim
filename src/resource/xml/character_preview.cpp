#include "character_preview.hpp"

#include <tinyxml2/tinyxml2.h>

#include "../../log.hpp"
#include "../../util/preferences.hpp"
#include "util.hpp"

#include <format>

using namespace tinyxml2;
using namespace game::util;

namespace game::resource::xml
{
  CharacterPreview::CharacterPreview(const std::filesystem::path& path)
  {
    XMLDocument document;

    physfs::Archive archive(path, path.filename().string());

    if (!archive.is_valid())
    {
      logger.error(std::format("Failed to initialize character preview from PhysicsFS archive: {} ({})", path.string(),
                               physfs::error_get()));
      return;
    }

    physfs::Path characterPath(archive + "/" + "character.xml");

    if (document_load(characterPath, document) != XML_SUCCESS) return;

    if (auto root = document.RootElement())
    {
      std::string textureRootPath{};
      query_string_attribute(root, "TextureRootPath", &textureRootPath);

      query_anm2(root, "Anm2", archive, textureRootPath, anm2, Anm2::NO_SOUNDS | Anm2::DEFAULT_ANIMATION_ONLY);
      query_texture(root, "Render", archive, textureRootPath, render);
      query_texture(root, "Portrait", archive, textureRootPath, portrait);

      query_string_attribute(root, "Name", &name);
      query_string_attribute(root, "Description", &description);
      query_string_attribute(root, "Credits", &credits);
      query_vec3(root, "ColorR", "ColorG", "ColorB", color);
      root->QueryFloatAttribute("Weight", &weight);

      if (auto element = root->FirstChildElement("Stages"))
        for (auto child = element->FirstChildElement("Stage"); child; child = child->NextSiblingElement("Stage"))
          stages++;
    }

    this->path = path;
    save = Save(save_path_get());
    isValid = true;

    logger.info(std::format("Initialized character preview: {}", name));
  }

  std::filesystem::path CharacterPreview::save_path_get()
  {
    auto savePath = path.stem();
    savePath = preferences::path() / "saves" / savePath.replace_extension(".save");
    std::filesystem::create_directories(savePath.parent_path());
    return savePath;
  }

  bool CharacterPreview::is_valid() const { return isValid; }
}
