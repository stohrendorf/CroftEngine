#include "util.h"

#include "engine/presenter.h"
#include "engine/world/world.h"
#include "menuobject.h"
#include "ui/label.h"

namespace menu
{
void rotateForSelection(MenuObject& object)
{
  if(object.rotationY == object.selectedRotationY)
    return;

  if(const auto dy = object.selectedRotationY - object.rotationY; dy > 0_deg && dy < 180_deg)
  {
    object.rotationY += 1024_au * 2;
  }
  else
  {
    object.rotationY -= 1024_au * 2;
  }
  object.rotationY -= object.rotationY % (1024_au * 2);
}

void idleRotation(engine::world::World& world, MenuObject& object)
{
  if(world.getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Null)
  {
    object.rotationY += 256_au * 2;
  }
}

void zeroRotation(MenuObject& object, const core::Angle& speed)
{
  BOOST_ASSERT(speed >= 0_deg);

  if(object.rotationY < 0_deg)
  {
    object.rotationY += speed * 2;
    if(object.rotationY > 0_deg)
      object.rotationY = 0_deg;
  }
  else if(object.rotationY > 0_deg)
  {
    object.rotationY -= speed * 2;
    if(object.rotationY < 0_deg)
      object.rotationY = 0_deg;
  }
}

constexpr int FadeSpeed = 30;

void resetMarks(ui::Label& label)
{
  label.backgroundAlpha = std::max(0, label.backgroundAlpha - FadeSpeed);
  label.outlineAlpha = std::max(0, label.outlineAlpha - FadeSpeed);
  label.backgroundGouraudAlpha = std::max(0, label.backgroundGouraudAlpha - FadeSpeed);
}

void markSelected(ui::Label& label)
{
  label.backgroundAlpha = std::min(255, label.backgroundAlpha + FadeSpeed);
  label.outlineAlpha = std::min(255, label.outlineAlpha + FadeSpeed);
  label.backgroundGouraudAlpha = std::min(0, label.backgroundGouraudAlpha + FadeSpeed);
}

std::unique_ptr<ui::Label> createFrame(const glm::ivec2& position, const glm::ivec2& size)
{
  auto result = std::make_unique<ui::Label>(position, " ");
  result->addBackground(size, {0, 0});
  result->backgroundGouraud = ui::Label::makeBackgroundCircle(gl::SRGB8{0, 255, 0}, 32, 0);
  result->backgroundGouraudAlpha = 255;
  result->outlineAlpha = 255;
  return result;
}

std::unique_ptr<ui::Label> createHeading(const std::string& heading, const glm::ivec2& position, const glm::ivec2& size)
{
  auto result = std::make_unique<ui::Label>(position, heading);
  result->addBackground(size, {0, 0});
  result->backgroundGouraud = ui::Label::makeBackgroundCircle(gl::SRGB8{32, 255, 112}, 96, 0);
  result->backgroundGouraudAlpha = 255;
  result->outlineAlpha = 255;
  return result;
}
} // namespace menu
