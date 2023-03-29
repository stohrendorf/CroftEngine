#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "qs/qs.h"
#include "serialization/serialization_fwd.h"

#include <cstddef>
#include <deque>
#include <gsl/gsl-lite.hpp>
#include <map>
#include <unordered_map>
#include <vector>

namespace engine::world
{
class World;
struct Box;
} // namespace engine::world

namespace engine::script
{
struct ObjectInfo;
}

namespace engine::ai
{
struct PathFinder
{
  [[nodiscard]] bool canVisit(const world::Box& box) const noexcept;

  [[nodiscard]] bool canVisit(const world::Box& box, bool ignoreBlocked, bool ignoreBlockable) const noexcept;

  /**
   * Sets #m_target to a random position within @p box with a margin of #Margin.
   */
  void setRandomSearchTarget(const gsl::not_null<const world::Box*>& box);

  /**
   * Calculates the next directly movable position from @p startPos and @p startBox, i.e. a position that's reachable
   * when moving straight towards it without collisions. @p startBox must contain @p startPos. @p moveTarget is an
   * output-only parameter and doesn't need to be initialised properly.
   *
   * @return @c true if a valid movement target was found, @c false otherwise.
   */
  bool calculateTarget(const world::World& world,
                       core::TRVec& moveTarget,
                       const core::TRVec& startPos,
                       const gsl::not_null<const world::Box*>& startBox);

  /**
   * Resets the search state and starts a new search for a path to @p box.
   */
  void setTargetBox(const gsl::not_null<const world::Box*>& box);

  void serialize(const serialization::Serializer<world::World>& ser) const;
  void deserialize(const serialization::Deserializer<world::World>& ser);

  void
    init(const world::World& world, const gsl::not_null<const world::Box*>& box, const script::ObjectInfo& objectInfo);

  void setLimits(const world::World& world,
                 const gsl::not_null<const world::Box*>& box,
                 const core::Length& step,
                 const core::Length& drop,
                 const core::Length& fly);

  //
  /**
   * Returns @c true if and only if the @p box is visited and marked unreachable.
   */
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

  [[nodiscard]] const auto& getTargetBox() const noexcept
  {
    return m_targetBox;
  }

  void setTarget(const core::TRVec& target) noexcept
  {
    m_target = target;
  }

  [[nodiscard]] bool isFlying() const noexcept
  {
    return m_fly != 0_len;
  }

  [[nodiscard]] const auto& getStep() const noexcept
  {
    return m_step;
  }

  [[nodiscard]] const auto& getDrop() const noexcept
  {
    return m_drop;
  }

  [[nodiscard]] const auto& getFly() const noexcept
  {
    return m_fly;
  }

private:
  void resetBoxes(const world::World& world, const gsl::not_null<const world::Box*>& box);

  /**
   * Does a limited expansion of the path graph and propagates reachability information.
   */
  void expandNodes(const world::World& world);

  std::vector<gsl::not_null<const world::Box*>> m_boxes;
  std::deque<gsl::not_null<const world::Box*>> m_expansions;
  std::unordered_map<gsl::not_null<const world::Box*>, bool> m_reachable;
  std::unordered_map<gsl::not_null<const world::Box*>, size_t> m_distances;
  std::unordered_map<gsl::not_null<const world::Box*>, gsl::not_null<const world::Box*>> m_edges;
  //! @brief The target box we need to reach
  const world::Box* m_targetBox = nullptr;
  core::TRVec m_target;

  bool m_cannotVisitBlocked = true;
  bool m_cannotVisitBlockable = false;

  //! @brief Movement limits.
  //! @warning Step and drop are negated.
  //! @{
  //! @brief Always positive, because of inverted Y axis.
  core::Length m_step = core::QuarterSectorSize;
  //! @brief Always negative, because of inverted Y axis.
  core::Length m_drop = -core::QuarterSectorSize;
  //! @brief Always positive, it's a limit.
  core::Length m_fly = 0_len;
  //! @}
};
} // namespace engine::ai
