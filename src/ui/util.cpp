#include "util.h"

#include "label.h"

namespace ui
{
std::unique_ptr<Label> createFrame(const glm::ivec2& position, const glm::ivec2& size)
{
  auto result = std::make_unique<Label>(position, " ");
  result->bgndSize = size;
  result->backgroundAlpha = 255;
  result->backgroundGouraud = Label::makeBackgroundCircle(gl::SRGB8{0, 255, 0}, 32, 0);
  result->backgroundGouraudAlpha = 255;
  result->outlineAlpha = 255;
  return result;
}

std::unique_ptr<Label> createHeading(const std::string& heading, const glm::ivec2& position, const glm::ivec2& size)
{
  auto result = std::make_unique<Label>(position, heading);
  result->bgndSize = size;
  result->backgroundAlpha = 255;
  result->backgroundGouraud = Label::makeBackgroundCircle(gl::SRGB8{32, 255, 112}, 96, 0);
  result->backgroundGouraudAlpha = 255;
  result->outlineAlpha = 255;
  return result;
}
} // namespace ui
