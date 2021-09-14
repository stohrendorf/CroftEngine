#pragma once

#include "engine/world/sprite.h"

#include <cstddef>
#include <cstdint>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <string>
#include <tuple>
#include <vector>

namespace ui
{
class Ui;

extern std::string makeAmmoString(const std::string& str);

class TRFont final
{
  gsl::span<const engine::world::Sprite> m_sprites;

public:
  explicit TRFont(const engine::world::SpriteSequence& sequence)
      : m_sprites{sequence.sprites}
  {
  }

  void draw(ui::Ui& ui, size_t sprite, const glm::ivec2& xy) const;
};

class Text
{
public:
  explicit Text(const std::string& text);

  void draw(Ui& ui, const TRFont& font, const glm::ivec2& position) const;

  [[nodiscard]] auto getWidth() const noexcept
  {
    return m_width;
  }

private:
  int m_width = 0;
  std::vector<std::tuple<glm::ivec2, uint8_t>> m_layout;
};
} // namespace ui
