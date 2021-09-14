#pragma once

#include "core/units.h"
#include "core/vec.h"
#include "serialization/serialization_fwd.h" // IWYU pragma: keep

#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <iosfwd>
#include <utility>

namespace engine::world
{
class World;
struct Room;
struct Sector;
} // namespace engine::world

namespace engine
{
struct Location final
{
  gsl::not_null<const world::Room*> room;

  core::TRVec position;

  explicit Location(gsl::not_null<const world::Room*> r, core::TRVec pos = {})
      : room{std::move(r)}
      , position{std::move(pos)}
  {
  }

  void serialize(const serialization::Serializer<world::World>& ser);
  [[nodiscard]] static Location create(const serialization::Serializer<world::World>& ser);

  gsl::not_null<const world::Sector*> updateRoom();

  [[nodiscard]] auto moved(const core::TRVec& d) const
  {
    auto tmp = *this;
    tmp.move(d);
    return tmp;
  }

  [[nodiscard]] auto moved(const core::Length& dx, const core::Length& dy, const core::Length& dz) const
  {
    return moved(core::TRVec{dx, dy, dz});
  }

  void move(const glm::vec3& d)
  {
    move(core::TRVec{d});
  }

  void move(const core::Length& dx, const core::Length& dy, const core::Length& dz)
  {
    move(core::TRVec{dx, dy, dz});
  }

  void move(const core::TRVec& d)
  {
    position += d;
  }

  [[nodiscard]] bool isValid() const;
};

extern std::ostream& operator<<(std::ostream& stream, const Location& rhs);
} // namespace engine
