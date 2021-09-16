#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "qs/qs.h"
#include "serialization/serialization_fwd.h"

#include <deque>
#include <gsl/gsl-lite.hpp>
#include <map>
#include <unordered_map>
#include <vector>

// IWYU pragma: no_forward_declare serialization::Serializer

namespace engine::world
{
class World;
struct Box;
} // namespace engine::world

namespace engine::ai
{
struct PathFinder
{
  static constexpr auto Margin = core::SectorSize / 2;

  bool cannotVisitBlocked = true;
  bool cannotVisitBlockable = false;

  [[nodiscard]] bool canVisit(const world::Box& box) const noexcept;

  //! @brief Movement limits
  //! @{
  core::Length step = core::QuarterSectorSize;
  core::Length drop = -core::QuarterSectorSize;
  core::Length fly = 0_len;
  //! @}

  core::TRVec target;

  void setRandomSearchTarget(const gsl::not_null<const world::Box*>& box);

  bool calculateTarget(const world::World& world,
                       core::TRVec& moveTarget,
                       const core::TRVec& startPos,
                       const gsl::not_null<const world::Box*>& startBox);

  void setTargetBox(const gsl::not_null<const world::Box*>& box);

  void serialize(const serialization::Serializer<world::World>& ser);

  void collectBoxes(const world::World& world, const gsl::not_null<const world::Box*>& box);

  // returns true if and only if the box is visited and marked unreachable
  [[nodiscard]] bool isUnreachable(const gsl::not_null<const world::Box*>& box) const
  {
    const auto it = m_reachable.find(box);
    return it != m_reachable.end() && !it->second;
  }

  [[nodiscard]] const gsl::not_null<const world::Box*>& getRandomBox() const;

  [[nodiscard]] const world::Box* getNextPathBox(const gsl::not_null<const world::Box*>& box) const
  {
    const auto it = m_edges.find(box);
    return it == m_edges.end() ? nullptr : it->second.get();
  }

  [[nodiscard]] const auto& getTargetBox() const
  {
    return m_targetBox;
  }

  [[nodiscard]] bool isFlying() const
  {
    return fly != 0_len;
  }

private:
  void searchPath(const world::World& world);

  std::vector<gsl::not_null<const world::Box*>> m_boxes;
  std::deque<gsl::not_null<const world::Box*>> m_expansions;
  std::unordered_map<gsl::not_null<const world::Box*>, bool> m_reachable;
  std::unordered_map<gsl::not_null<const world::Box*>, gsl::not_null<const world::Box*>> m_edges;
  //! @brief The target box we need to reach
  const world::Box* m_targetBox = nullptr;
};
} // namespace engine::ai
