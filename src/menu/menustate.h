#pragma once

#include <gl/image.h>
#include <memory>

namespace engine
{
class World;
}

namespace menu
{
struct MenuDisplay;
struct MenuObject;
struct MenuRingTransform;

class MenuState
{
protected:
  const std::shared_ptr<MenuRingTransform> m_ringTransform;

public:
  explicit MenuState(std::shared_ptr<MenuRingTransform> ringTransform)
      : m_ringTransform{std::move(ringTransform)}
  {
  }

  virtual ~MenuState() = default;

  virtual void begin(engine::World& /*world*/)
  {
  }
  virtual void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) = 0;
  virtual std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display)
    = 0;

  template<typename T, typename... Ts>
  auto create(Ts&&... args) -> std::enable_if_t<std::is_base_of_v<MenuState, T>, std::unique_ptr<T>>
  {
    return std::make_unique<T>(m_ringTransform, std::forward<Ts>(args)...);
  }
};
} // namespace menu
