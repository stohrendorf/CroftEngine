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
  virtual void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) = 0;
  virtual std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) = 0;

  template<typename T, typename... Ts>
  auto create(Ts&&... args) -> std::enable_if_t<std::is_base_of_v<MenuState, T>, std::unique_ptr<T>>
  {
    return std::make_unique<T>(m_ringTransform, std::forward<Ts>(args)...);
  }
};
} // namespace menu
