#pragma once

#include "core/angle.h"
#include "core/units.h"
#include "menuobject.h"

#include <cstddef>
#include <cstdint>
#include <gsl/gsl-lite.hpp>
#include <string>
#include <utility>
#include <vector>

namespace menu
{
struct MenuRing
{
  enum class Type : uint8_t
  {
    Inventory,
    Options,
    Items
  };

  std::string title;
  Type type;
  std::vector<MenuObject> list;
  size_t currentObject = 0;

  explicit MenuRing(Type type, std::string title, std::vector<MenuObject> list)
      : title{std::move(title)}
      , type{type}
      , list{std::move(list)}
  {
  }

  [[nodiscard]] auto getAnglePerItem() const
  {
    gsl_Expects(!list.empty());
    const auto anglePerItemDeg = 360.0f / gsl::narrow_cast<float>(list.size());
    return core::angleFromDegrees(anglePerItemDeg);
  }

  MenuObject& getSelectedObject()
  {
    return list.at(currentObject);
  }

  [[nodiscard]] core::Angle getCurrentObjectAngle() const
  {
    return getAnglePerItem() * gsl::narrow_cast<core::Angle::type>(currentObject);
  }
};
} // namespace menu
