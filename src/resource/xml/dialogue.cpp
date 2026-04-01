#include "dialogue.hpp"
#include "util.hpp"

#include "../../log.hpp"
#include "../../util/math.hpp"

#include <algorithm>
#include <format>

using namespace tinyxml2;
using namespace game::util;

namespace game::resource::xml
{
  void Dialogue::query_entry_id(XMLElement* element, const char* name, int& id)
  {
    std::string entryID{};
    query_string_attribute(element, name, &entryID);
    if (entryIDMap.contains(entryID))
      id = entryIDMap.at(entryID);
    else if (entryID.empty())
      entryID = -1;
    else
    {
      logger.warning("Dialogue entries does not contain: " + entryID);
      id = -1;
    }
  }

  void Dialogue::query_pool_id(XMLElement* element, const char* name, int& id)
  {
    std::string poolID{};
    query_string_attribute(element, name, &poolID);
    if (poolMap.contains(poolID))
      id = poolMap.at(poolID);
    else if (poolID.empty())
      poolID = -1;
    else
    {
      logger.warning("Dialogue pools does not contain: " + poolID);
      id = -1;
    }
  }

  Dialogue::Dialogue(const physfs::Path& path)
  {

    XMLDocument document;

    if (document_load(path, document) != XML_SUCCESS) return;

    if (auto root = document.RootElement())
    {
      if (auto element = root->FirstChildElement("Entries"))
      {
        int id{};

        for (auto child = element->FirstChildElement("Entry"); child; child = child->NextSiblingElement("Entry"))
        {
          std::string stringID{};
          query_string_attribute(child, "ID", &stringID);
          entryIDMap.emplace(stringID, id);
          entryIDMapReverse.emplace(id, stringID);
          id++;
        }

        id = 0;

        for (auto child = element->FirstChildElement("Entry"); child; child = child->NextSiblingElement("Entry"))
        {
          Entry entry{};

          entry.name = entryIDMapReverse.at(id);

          query_string_attribute(child, "Text", &entry.text);

          query_string_attribute(child, "Animation", &entry.animation);

          if (child->FindAttribute("Next"))
          {
            std::string nextID{};
            query_string_attribute(child, "Next", &nextID);
            if (!entryIDMap.contains(nextID))
              logger.warning(std::format("Dialogue: next ID does not point to a valid Entry! ({})", nextID));
            else
              entry.nextID = entryIDMap.at(nextID);
          }

          for (auto choiceChild = child->FirstChildElement("Choice"); choiceChild;
               choiceChild = choiceChild->NextSiblingElement("Choice"))
          {
            Choice choice{};
            query_entry_id(choiceChild, "Next", choice.nextID);
            query_string_attribute(choiceChild, "Text", &choice.text);
            entry.choices.emplace_back(std::move(choice));
          }

          entries.emplace_back(std::move(entry));

          id++;
        }

        entrySelectionOrder.assign(entries.size(), -1);
      }

      if (auto element = root->FirstChildElement("Pools"))
      {
        int id{};

        for (auto child = element->FirstChildElement("Pool"); child; child = child->NextSiblingElement("Pool"))
        {
          Pool pool{};
          std::string stringID{};
          query_string_attribute(child, "ID", &stringID);
          poolMap[stringID] = id;
          pools.emplace_back(pool);
          id++;
        }

        id = 0;

        for (auto child = element->FirstChildElement("Pool"); child; child = child->NextSiblingElement("Pool"))
        {
          auto& pool = pools.at(id);

          for (auto entryChild = child->FirstChildElement("PoolEntry"); entryChild;
               entryChild = entryChild->NextSiblingElement("PoolEntry"))
          {
            int entryID{};
            query_entry_id(entryChild, "ID", entryID);
            pool.emplace_back(entryID);
          }

          id++;
        }

        logger.info(std::format("Initialized dialogue: {}", path.c_str()));
        isValid = true;
      }

      if (auto element = root->FirstChildElement("Start"))
      {
        query_entry_id(element, "ID", start.id);
        query_string_attribute(element, "Animation", &start.animation);
      }
      if (auto element = root->FirstChildElement("End")) query_entry_id(element, "ID", end.id);
      if (auto element = root->FirstChildElement("Help")) query_entry_id(element, "ID", help.id);

      if (auto element = root->FirstChildElement("Digest")) query_pool_id(element, "PoolID", digest.id);
      if (auto element = root->FirstChildElement("Eat")) query_pool_id(element, "PoolID", eat.id);
      if (auto element = root->FirstChildElement("EatFull")) query_pool_id(element, "PoolID", eatFull.id);
      if (auto element = root->FirstChildElement("Feed")) query_pool_id(element, "PoolID", feed.id);
      if (auto element = root->FirstChildElement("FeedFull")) query_pool_id(element, "PoolID", feedFull.id);
      if (auto element = root->FirstChildElement("FoodTaken")) query_pool_id(element, "PoolID", foodTaken.id);
      if (auto element = root->FirstChildElement("FoodTakenFull")) query_pool_id(element, "PoolID", foodTakenFull.id);
      if (auto element = root->FirstChildElement("Full")) query_pool_id(element, "PoolID", full.id);
      if (auto element = root->FirstChildElement("LowCapacity")) query_pool_id(element, "PoolID", lowCapacity.id);
      if (auto element = root->FirstChildElement("Random")) query_pool_id(element, "PoolID", random.id);
      if (auto element = root->FirstChildElement("Throw")) query_pool_id(element, "PoolID", throw_.id);
      if (auto element = root->FirstChildElement("StageUp")) query_pool_id(element, "PoolID", stageUp.id);
    }
  }

  int Dialogue::entry_pick(Pool& pool)
  {
    if (pool.empty()) return -1;

    std::vector<int> unselected{};
    for (auto id : pool)
      if (id >= 0 && id < (int)entrySelectionOrder.size() && entrySelectionOrder[id] < 0) unselected.emplace_back(id);

    std::vector<int> candidates = unselected.empty() ? std::vector<int>(pool.begin(), pool.end()) : unselected;
    if (candidates.empty()) return -1;

    auto oldestOrder = entrySelectionOrder[candidates.front()];
    for (auto id : candidates)
      oldestOrder = std::min(oldestOrder, entrySelectionOrder[id]);

    std::vector<int> oldestCandidates{};
    for (auto id : candidates)
      if (entrySelectionOrder[id] == oldestOrder) oldestCandidates.emplace_back(id);

    auto pickedID = oldestCandidates.at(rand() % oldestCandidates.size());
    entrySelectionOrder[pickedID] = selectionCounter++;
    return pickedID;
  }
  Dialogue::Entry* Dialogue::get(int id) { return &entries.at(id); }
  Dialogue::Entry* Dialogue::get(Dialogue::EntryReference& entry) { return &entries.at(entry.id); }
  Dialogue::Entry* Dialogue::get(const std::string& string) { return &entries.at(entryIDMap.at(string)); }
  Dialogue::Entry* Dialogue::get(Dialogue::PoolReference& pool) { return &entries.at(entry_pick(pools.at(pool.id))); }
  Dialogue::Entry* Dialogue::get(Dialogue::Pool& pool) { return &entries.at(entry_pick(pool)); }
}
