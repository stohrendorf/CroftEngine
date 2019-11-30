#pragma once

#include "core/id.h"
#include "gsl-lite.hpp"

#include <map>

namespace engine
{
class Engine;

class Inventory
{
private:
  Engine& m_engine;
  std::map<TR1ItemId, size_t> m_inventory;

public:
  explicit Inventory(Engine& engine)
      : m_engine{engine}
  {
  }

  void serialize(const serialization::Serializer& ser);

  void put(core::TypeId id, size_t quantity = 1);

  bool tryTake(TR1ItemId id, size_t quantity = 1);

  [[nodiscard]] size_t count(const TR1ItemId id) const
  {
    const auto it = m_inventory.find(id);
    if(it == m_inventory.end())
      return 0;

    return it->second;
  }

  void clear()
  {
    m_inventory.clear();
  }

  bool tryUse(TR1ItemId id);
};
} // namespace engine
