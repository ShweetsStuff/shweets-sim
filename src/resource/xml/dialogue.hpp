#pragma once

#include <tinyxml2.h>

#include <string>

#include <map>
#include <vector>

#include "../../util/physfs.hpp"

namespace game::resource::xml
{
  class Dialogue
  {
  public:
    struct Choice
    {
      std::string text{};
      int nextID{-1};
    };

    struct Entry
    {
      std::string name{};
      std::string animation{};
      std::string text{};
      std::vector<Choice> choices{};
      int nextID{-1};

      inline bool is_last() const { return choices.empty() && nextID == -1; };
    };

    struct EntryReference
    {
      int id{-1};
      std::string animation{};

      inline bool is_valid() const { return id != -1; };
    };

    class PoolReference
    {
    public:
      int id{-1};
      inline bool is_valid() const { return id != -1; };
    };

    class Pool : public std::vector<int> {};

    std::map<std::string, int> entryIDMap;
    std::map<int, std::string> entryIDMapReverse;
    std::vector<Entry> entries{};

    std::vector<Pool> pools{};
    std::map<std::string, int> poolMap{};
    std::vector<long long> entrySelectionOrder{};
    long long selectionCounter{};

    EntryReference start{-1};
    EntryReference end{-1};
    EntryReference help{-1};
    PoolReference digest{-1};
    PoolReference eatFull{-1};
    PoolReference eat{-1};
    PoolReference feedFull{-1};
    PoolReference feed{-1};
    PoolReference foodTakenFull{-1};
    PoolReference foodTaken{-1};
    PoolReference full{-1};
    PoolReference random{-1};
    PoolReference lowCapacity{-1};
    PoolReference throw_{-1};
    PoolReference stageUp{-1};

    bool isValid{};

    Dialogue() = default;
    Dialogue(const util::physfs::Path&);
    Entry* get(const std::string&);
    Entry* get(int id);
    Entry* get(Dialogue::EntryReference&);
    Entry* get(Dialogue::Pool&);
    Entry* get(Dialogue::PoolReference&);
    int entry_pick(Pool&);
    void query_entry_id(tinyxml2::XMLElement* element, const char* name, int& id);
    void query_pool_id(tinyxml2::XMLElement* element, const char* name, int& id);
    inline bool is_valid() const { return isValid; };
  };
}
