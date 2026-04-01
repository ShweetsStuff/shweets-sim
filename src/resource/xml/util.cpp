#include "util.hpp"

#include <format>

#include "../../util/physfs.hpp"
#include "../../util/string.hpp"

#include "../../log.hpp"

using namespace tinyxml2;
using namespace game::util;

namespace game::resource::xml
{
  XMLError query_result_merge(XMLError result, XMLError next) { return result == XML_SUCCESS ? next : result; }

  XMLError query_string_attribute(XMLElement* element, const char* attribute, std::string* value)
  {
    const char* temp = nullptr;
    auto result = element->QueryStringAttribute(attribute, &temp);
    if (result == XML_SUCCESS && temp && value) *value = temp;
    return result;
  }

  XMLError query_bool_attribute(XMLElement* element, const char* attribute, bool* value)
  {
    std::string temp{};
    auto result = query_string_attribute(element, attribute, &temp);
    temp = string::to_lower(temp);
    if (value) *value = temp == "true" || temp == "1" ? true : false;
    return result;
  }

  XMLError query_path_attribute(XMLElement* element, const char* attribute, std::filesystem::path* value)
  {
    std::string temp{};
    auto result = query_string_attribute(element, attribute, &temp);
    if (value) *value = std::filesystem::path(temp);
    return result;
  }

  XMLError query_color_attribute(XMLElement* element, const char* attribute, float* value)
  {
    int temp{};
    auto result = element->QueryIntAttribute(attribute, &temp);
    if (result == XML_SUCCESS && value) *value = (temp / 255.0f);
    return result;
  }

  XMLError query_ivec2(XMLElement* element, const char* attributeX, const char* attributeY, glm::ivec2& value)
  {
    auto result = element->QueryIntAttribute(attributeX, &value.x);
    result = query_result_merge(result, element->QueryIntAttribute(attributeY, &value.y));
    return result;
  }

  XMLError query_vec2(XMLElement* element, const char* attributeX, const char* attributeY, glm::vec2& value)
  {
    auto result = element->QueryFloatAttribute(attributeX, &value.x);
    result = query_result_merge(result, element->QueryFloatAttribute(attributeY, &value.y));
    return result;
  }

  XMLError query_vec3(XMLElement* element, const char* attributeX, const char* attributeY, const char* attributeZ,
                      glm::vec3& value)
  {
    auto result = element->QueryFloatAttribute(attributeX, &value.x);
    result = query_result_merge(result, element->QueryFloatAttribute(attributeY, &value.y));
    result = query_result_merge(result, element->QueryFloatAttribute(attributeZ, &value.z));
    return result;
  }

  XMLError set_bool_attribute(XMLElement* element, const char* attribute, bool value)
  {
    element->SetAttribute(attribute, value ? "true" : "false");
    return XML_SUCCESS;
  }

  XMLError set_ivec2_attribute(XMLElement* element, const char* attributeX, const char* attributeY,
                               const glm::ivec2& value)
  {
    element->SetAttribute(attributeX, value.x);
    element->SetAttribute(attributeY, value.y);
    return XML_SUCCESS;
  }

  XMLError set_vec2_attribute(XMLElement* element, const char* attributeX, const char* attributeY,
                              const glm::vec2& value)
  {
    element->SetAttribute(attributeX, value.x);
    element->SetAttribute(attributeY, value.y);
    return XML_SUCCESS;
  }

  XMLError set_vec3_attribute(XMLElement* element, const char* attributeX, const char* attributeY,
                              const char* attributeZ, const glm::vec3& value)
  {
    element->SetAttribute(attributeX, value.x);
    element->SetAttribute(attributeY, value.y);
    element->SetAttribute(attributeZ, value.z);
    return XML_SUCCESS;
  }

  XMLError query_float_optional_attribute(XMLElement* element, const char* attribute, std::optional<float>& value)
  {
    value.emplace();
    auto result = element->QueryFloatAttribute(attribute, &*value);
    if (result == XML_NO_ATTRIBUTE) value.reset();
    return result;
  }

  XMLError query_int_optional_attribute(XMLElement* element, const char* attribute, std::optional<int>& value)
  {
    value.emplace();
    auto result = element->QueryIntAttribute(attribute, &*value);
    if (result == XML_NO_ATTRIBUTE) value.reset();
    return result;
  }

  XMLError query_optional_vec3(XMLElement* element, const char* attributeX, const char* attributeY,
                               const char* attributeZ, std::optional<glm::vec3>& value)
  {
    auto hasX = element->FindAttribute(attributeX);
    auto hasY = element->FindAttribute(attributeY);
    auto hasZ = element->FindAttribute(attributeZ);

    if (!hasX && !hasY && !hasZ)
    {
      value.reset();
      return XML_NO_ATTRIBUTE;
    }

    value = glm::vec3();
    auto result = XML_SUCCESS;

    if (hasX) result = query_result_merge(result, element->QueryFloatAttribute(attributeX, &value->x));
    if (hasY) result = query_result_merge(result, element->QueryFloatAttribute(attributeY, &value->y));
    if (hasZ) result = query_result_merge(result, element->QueryFloatAttribute(attributeZ, &value->z));

    return result;
  }

  XMLError document_load(const physfs::Path& path, XMLDocument& document)
  {
    if (!path.is_valid())
    {
      logger.error(std::format("Failed to open XML document: {} ({})", path.c_str(), physfs::error_get()));
      return XML_ERROR_FILE_NOT_FOUND;
    }

    auto buffer = path.read();

    if (buffer.empty())
    {
      logger.error(std::format("Failed to read XML document: {} ({})", path.c_str(), physfs::error_get()));
      return XML_ERROR_FILE_COULD_NOT_BE_OPENED;
    }

    auto result = document.Parse((const char*)buffer.data(), buffer.size());
    if (result != XML_SUCCESS)
      logger.error(std::format("Failed to parse XML document: {} ({})", path.c_str(), document.ErrorStr()));

    return result;
  }

  XMLError query_event_id(XMLElement* element, const char* name, const Anm2& anm2, int& eventID)
  {
    std::string string{};
    auto result = query_string_attribute(element, name, &string);
    if (result != XML_SUCCESS) return result;

    if (anm2.eventMap.contains(string))
    {
      eventID = anm2.eventMap.at(string);
      return XML_SUCCESS;
    }
    else
    {
      logger.error(std::format("Could not query anm2 event ID: {} ({})", string, anm2.path));
      eventID = -1;
      return XML_ERROR_PARSING_ATTRIBUTE;
    }
  }

  XMLError query_layer_id(XMLElement* element, const char* name, const Anm2& anm2, int& layerID)
  {
    std::string string{};
    auto result = query_string_attribute(element, name, &string);
    if (result != XML_SUCCESS) return result;

    if (anm2.layerMap.contains(string))
    {
      layerID = anm2.layerMap.at(string);
      return XML_SUCCESS;
    }
    else
    {
      logger.error(std::format("Could not query anm2 layer ID: {} ({})", string, anm2.path));
      layerID = -1;
      return XML_ERROR_PARSING_ATTRIBUTE;
    }
  }

  XMLError query_null_id(XMLElement* element, const char* name, const Anm2& anm2, int& nullID)
  {
    std::string string{};
    auto result = query_string_attribute(element, name, &string);
    if (result != XML_SUCCESS) return result;

    if (anm2.nullMap.contains(string))
    {
      nullID = anm2.nullMap.at(string);
      return XML_SUCCESS;
    }
    else
    {
      logger.error(std::format("Could not query anm2 null ID: {} ({})", string, anm2.path));
      nullID = -1;
      return XML_ERROR_PARSING_ATTRIBUTE;
    }
  }

  XMLError query_anm2(XMLElement* element, const char* name, const std::string& archive, const std::string& rootPath,
                      Anm2& anm2, Anm2::Flags flags)
  {
    std::string string{};
    auto result = query_string_attribute(element, name, &string);
    if (result != XML_SUCCESS) return result;

    anm2 = Anm2(physfs::Path(archive + "/" + rootPath + "/" + string), flags);
    return XML_SUCCESS;
  }

  XMLError query_texture(XMLElement* element, const char* name, const std::string& archive, const std::string& rootPath,
                         Texture& texture)
  {
    std::string string{};
    auto result = query_string_attribute(element, name, &string);
    if (result != XML_SUCCESS) return result;

    texture = Texture(physfs::Path(archive + "/" + rootPath + "/" + string));
    return XML_SUCCESS;
  }

  XMLError query_sound(XMLElement* element, const char* name, const std::string& archive, const std::string& rootPath,
                       Audio& sound)
  {
    std::string string{};
    auto result = query_string_attribute(element, name, &string);
    if (result != XML_SUCCESS) return result;

    sound = Audio(physfs::Path(archive + "/" + rootPath + "/" + string));
    return XML_SUCCESS;
  }

  XMLError query_font(XMLElement* element, const char* name, const std::string& archive, const std::string& rootPath,
                      Font& font)
  {
    std::string string{};
    auto result = query_string_attribute(element, name, &string);
    if (result != XML_SUCCESS) return result;

    font = Font(physfs::Path(archive + "/" + rootPath + "/" + string));
    return XML_SUCCESS;
  }

  XMLError query_animation_entry(XMLElement* element, AnimationEntry& animationEntry)
  {
    auto result = query_string_attribute(element, "Animation", &animationEntry.animation);
    result = query_result_merge(result, element->QueryFloatAttribute("Weight", &animationEntry.weight));
    return result;
  }

  XMLError query_animation_entry_collection(XMLElement* element, const char* name,
                                            AnimationEntryCollection& animationEntryCollection)
  {
    auto result = XML_SUCCESS;
    for (auto child = element->FirstChildElement(name); child; child = child->NextSiblingElement(name))
      result = query_result_merge(result, query_animation_entry(child, animationEntryCollection.emplace_back()));
    return result;
  }

  XMLError query_sound_entry(XMLElement* element, const std::string& archive, const std::string& rootPath,
                             SoundEntry& soundEntry, const std::string& attributeName)
  {
    auto result = query_sound(element, attributeName.c_str(), archive, rootPath, soundEntry.sound);
    result = query_result_merge(result, element->QueryFloatAttribute("Weight", &soundEntry.weight));
    return result;
  }

  XMLError query_sound_entry_collection(XMLElement* element, const char* name, const std::string& archive,
                                        const std::string& rootPath, SoundEntryCollection& soundEntryCollection,
                                        const std::string& attributeName)
  {
    auto result = XML_SUCCESS;
    for (auto child = element->FirstChildElement(name); child; child = child->NextSiblingElement(name))
      result = query_result_merge(
          result, query_sound_entry(child, archive, rootPath, soundEntryCollection.emplace_back(), attributeName));
    return result;
  }
}
