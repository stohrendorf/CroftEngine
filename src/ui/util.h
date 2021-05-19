#pragma once

#include <glm/fwd.hpp>
#include <memory>

namespace ui
{
struct Label;

extern std::unique_ptr<Label> createFrame(const glm::ivec2& position, const glm::ivec2& size);
extern std::unique_ptr<Label>
  createHeading(const std::string& heading, const glm::ivec2& position, const glm::ivec2& size);
} // namespace ui
