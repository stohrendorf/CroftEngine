#pragma once

#include <memory>
#include <type_traits>
#include <utility>

namespace engine::world
{
class World;
}

namespace ui
{
class Ui;
}

namespace menu
{
struct MenuDisplay;
struct MenuObject;
struct MenuRingTransform;

class MenuState
{
protected:
  std::shared_ptr<MenuRingTransform> m_ringTransform;

public:
  explicit MenuState(std::shared_ptr<MenuRingTransform> ringTransform)
      : m_ringTransform{std::move(ringTransform)}
  {
  }

  virtual ~MenuState() = default;

  virtual void begin(engine::world::World& /*world*/)
  {
  }
  virtual void handleObjectTick(engine::world::World& world, MenuDisplay& display, MenuObject& object) = 0;
  virtual std::unique_ptr<MenuState> tick(engine::world::World& world, MenuDisplay& display) = 0;
  virtual void constructUi(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) = 0;

  template<std::derived_from<MenuState> T, typename... Ts>
  std::unique_ptr<T> create(Ts&&... args)
  {
    return std::make_unique<T>(m_ringTransform, std::forward<Ts>(args)...);
  }
};
} // namespace menu
