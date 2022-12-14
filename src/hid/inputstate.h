#pragma once

#include "actions.h"

#include <boost/container/flat_map.hpp>

namespace hid
{
enum class AxisMovement
{
  Positive,
  Right = Positive,
  Forward = Positive,
  Null,
  Negative,
  Left = Negative,
  Backward = Negative
};

struct InputState
{
  template<typename T>
  struct DebouncedValue
  {
    T current;
    T previous;

    explicit DebouncedValue(const T& initial = {})
        : current{initial}
        , previous{initial}
    {
    }

    DebouncedValue<T>& operator=(const T& state)
    {
      previous = std::exchange(current, state);
      return *this;
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator T() const
    {
      return current;
    }

    [[nodiscard]] bool justChanged() const
    {
      return current != previous;
    }

    [[nodiscard]] bool justChangedTo(const T& value) const
    {
      return justChanged() && current == value;
    }
  };

  using Button = DebouncedValue<bool>;
  using Axis = DebouncedValue<AxisMovement>;

  Axis xMovement{AxisMovement::Null};
  Axis menuXMovement{AxisMovement::Null};
  Axis zMovement{AxisMovement::Null};
  Axis menuZMovement{AxisMovement::Null};
  Axis stepMovement{AxisMovement::Null};

  std::vector<std::pair<Action, Button>> actions{
    {Action::Jump, Button{false}},
    {Action::Walk, Button{false}},
    {Action::Roll, Button{false}},
    {Action::Action, Button{false}},
    {Action::FreeLook, Button{false}},
    {Action::Holster, Button{false}},
    {Action::Menu, Button{false}},
    {Action::DrawPistols, Button{false}},
    {Action::DrawShotgun, Button{false}},
    {Action::DrawUzis, Button{false}},
    {Action::DrawMagnums, Button{false}},
    {Action::ConsumeSmallMedipack, Button{false}},
    {Action::ConsumeLargeMedipack, Button{false}},
    {Action::Save, Button{false}},
    {Action::Load, Button{false}},
#ifndef NDEBUG
    {Action::CheatDive, Button{false}},
#endif
  };

  void setXAxisMovement(const bool left, const bool right)
  {
    if(!left && right)
      xMovement = AxisMovement::Right;
    else if(left && !right)
      xMovement = AxisMovement::Left;
    else
      xMovement = AxisMovement::Null;
  }

  void setMenuXAxisMovement(const bool left, const bool right)
  {
    if(!left && right)
      menuXMovement = AxisMovement::Right;
    else if(left && !right)
      menuXMovement = AxisMovement::Left;
    else
      menuXMovement = AxisMovement::Null;
  }

  void setStepMovement(const bool left, const bool right)
  {
    if(!left && right)
      stepMovement = AxisMovement::Right;
    else if(left && !right)
      stepMovement = AxisMovement::Left;
    else
      stepMovement = AxisMovement::Null;
  }

  void setZAxisMovement(const bool back, const bool forward)
  {
    if(!back && forward)
      zMovement = AxisMovement::Forward;
    else if(back && !forward)
      zMovement = AxisMovement::Backward;
    else
      zMovement = AxisMovement::Null;
  }

  void setMenuZAxisMovement(const bool back, const bool forward)
  {
    if(!back && forward)
      menuZMovement = AxisMovement::Forward;
    else if(back && !forward)
      menuZMovement = AxisMovement::Backward;
    else
      menuZMovement = AxisMovement::Null;
  }
};
} // namespace hid
