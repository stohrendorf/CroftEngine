#pragma once

#include "core/angle.h"

namespace hid
{
namespace
{
const core::Angle FreeLookMouseMovementScale{50_deg};
}

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

    explicit DebouncedValue(const T& initial)
        : current{initial}
        , previous{initial}
    {
    }

    DebouncedValue<T>& operator=(const T& state)
    {
      previous = std::exchange(current, state);
      return *this;
    }

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

    [[nodiscard]] bool justChangedFrom(const T& value) const
    {
      return justChanged() && current != value;
    }
  };

  using Button = DebouncedValue<bool>;
  using Axis = DebouncedValue<AxisMovement>;

  Axis xMovement{AxisMovement::Null};
  Axis zMovement{AxisMovement::Null};
  Axis stepMovement{AxisMovement::Null};

  Button jump{false};
  Button moveSlow{false};
  Button roll{false};
  Button action{false};
  Button freeLook{false};
  Button debug{false};
  Button crt{false};
  Button holster{false};
  Button menu{false};
  Button _1{false};
  Button _2{false};
  Button _3{false};
  Button _4{false};
  Button _5{false};
  Button _6{false};
  Button save{false};
  Button load{false};
  glm::vec2 mouseMovement;

  void setXAxisMovement(const bool left, const bool right)
  {
    if(left < right)
      xMovement = AxisMovement::Right;
    else if(left > right)
      xMovement = AxisMovement::Left;
    else
      xMovement = AxisMovement::Null;
  }

  void setStepMovement(const bool left, const bool right)
  {
    if(left < right)
      stepMovement = AxisMovement::Right;
    else if(left > right)
      stepMovement = AxisMovement::Left;
    else
      stepMovement = AxisMovement::Null;
  }

  void setZAxisMovement(const bool back, const bool forward)
  {
    if(back < forward)
      zMovement = AxisMovement::Forward;
    else if(back > forward)
      zMovement = AxisMovement::Backward;
    else
      zMovement = AxisMovement::Null;
  }
};
} // namespace hid
