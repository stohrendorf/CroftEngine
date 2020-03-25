#pragma once

#include "core/id.h"

#include <gsl-lite.hpp>
#include <map>

namespace engine
{
namespace objects
{
class LaraObject;
}

class Inventory
{
private:
  std::map<TR1ItemId, size_t> m_inventory;

public:
  explicit Inventory() = default;

  void serialize(const serialization::Serializer& ser);

  void put(objects::LaraObject& lara, core::TypeId id, size_t quantity = 1);

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

  bool tryUse(objects::LaraObject& lara, TR1ItemId id);
};
} // namespace engine
