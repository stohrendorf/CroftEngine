#pragma once

#include "core/vec.h"

#include <cstdint>

namespace engine::world
{
class World;

struct CameraSink
{
  core::TRVec position{};

  union
  {
    uint16_t room{};
    uint16_t underwaterCurrentStrength;
  };

  union
  {
    // TODO mutable flags
    mutable uint16_t flags{};
    uint16_t box_index;
  };

  void serialize(const serialization::Serializer<engine::world::World>& ser);

  constexpr bool isActive() const noexcept
  {
    return (flags & 1u) != 0;
  }

  void setActive(const bool flg) const noexcept
  {
    if(flg)
      flags |= 1u;
    else
      flags &= ~1u;
  }
};
} // namespace engine::world