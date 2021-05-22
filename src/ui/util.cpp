#include "util.h"

#include "label.h"

namespace ui
{
std::unique_ptr<Label> createFrame(const glm::ivec2& position, const glm::ivec2& size)
{
  auto result = std::make_unique<Label>(position, " ");
  result->bgndSize = size;
  result->backgroundGouraud = Label::makeBackgroundCircle(gl::SRGBA8{0, 128, 0, 192}, gl::SRGBA8{0, 0, 0, 192});
  result->backgroundGouraudAlpha = 255;
  result->outlineAlpha = 255;
  return result;
}

std::unique_ptr<Label> createHeading(const std::string& heading, const glm::ivec2& position, const glm::ivec2& size)
{
  auto result = std::make_unique<Label>(position, heading);
  result->bgndSize = size;
  result->backgroundAlpha = 255;
  result->outlineAlpha = 255;
  return result;
}
} // namespace ui
