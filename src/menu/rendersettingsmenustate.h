#pragma once

#include "listdisplaymenustate.h"

#include <functional>

namespace ui
{
struct Label;
}

namespace engine
{
class Engine;
}

namespace menu
{
class RenderSettingsMenuState : public ListDisplayMenuState
{
private:
  std::unique_ptr<MenuState> m_previous;
  std::vector<std::tuple<std::function<bool()>, std::function<void()>>> m_handlers;

public:
  explicit RenderSettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                   std::unique_ptr<MenuState> previous,
                                   engine::Engine& engine);

  std::unique_ptr<MenuState> onSelected(size_t idx, engine::world::World& world, MenuDisplay& display) override;
  std::unique_ptr<MenuState> onAborted() override;
};
} // namespace menu
