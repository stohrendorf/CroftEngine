#pragma once

#include "loader/file/texture.h"

namespace engine::world
{
struct AtlasTile
{
  loader::file::TextureKey textureKey;
  std::array<glm::vec2, 4> uvCoordinates{};
  bool isOpaque = false;

  bool operator==(const AtlasTile& rhs) const
  {
    return textureKey == rhs.textureKey && uvCoordinates == rhs.uvCoordinates;
  }

  bool operator<(const AtlasTile& rhs) const
  {
    if(!(textureKey == rhs.textureKey))
      return textureKey < rhs.textureKey;

    return getArea() < rhs.getArea();
  }

  [[nodiscard]] std::pair<glm::vec2, glm::vec2> getMinMaxUv() const
  {
    glm::vec2 xy0{std::numeric_limits<glm::float32>::max()};
    glm::vec2 xy1{std::numeric_limits<glm::float32>::min()};
    bool allInvalid = true;
    for(const auto& uvComponent : uvCoordinates)
    {
      if(uvComponent.x == 0 && uvComponent.y == 0)
        continue;

      allInvalid = false;
      xy0 = glm::min(uvComponent, xy0);
      xy1 = glm::max(uvComponent, xy1);
    }
    if(allInvalid)
      return {{0.0f, 0.0f}, {0.0f, 0.0f}};
    else
      return {xy0, xy1};
  }

  [[nodiscard]] float getArea() const
  {
    const auto [min, max] = getMinMaxUv();
    const auto dims = glm::abs(max - min);
    return dims.x * dims.y;
  }
};
} // namespace engine::world
