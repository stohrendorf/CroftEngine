#pragma once

#include "engine/items_tr1.h"
#include "engine/lighting.h"
#include "menuringtransform.h"
#include "ui/text.h"

#include <cstddef>
#include <gl/buffer.h>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace engine::world
{
class World;
}

namespace ui
{
class Ui;
}

namespace render::scene
{
class Material;
}

namespace render::pass
{
class Framebuffer;
}

namespace menu
{
enum class InventoryMode
{
  GameMode,
  TitleMode,
  SaveMode,
  LoadMode,
  DeathMode
};

struct MenuObject;
struct MenuRing;
class MenuState;

enum class MenuResult
{
  None,
  Closed,
  ExitToTitle,
  ExitGame,
  NewGame,
  LaraHome,
  RequestLoad
};

struct MenuDisplay
{
  explicit MenuDisplay(InventoryMode mode, engine::world::World& world, const glm::ivec2& viewport);
  ~MenuDisplay();

  const InventoryMode mode;
  std::optional<engine::TR1ItemId> inventoryChosen{};
  bool allowMenuClose = true;
  bool allowSave = true;

  std::shared_ptr<MenuRingTransform> ringTransform = std::make_shared<MenuRingTransform>();
  std::unique_ptr<MenuState> m_currentState;

  void display(ui::Ui& ui, engine::world::World& world);
  MenuResult result = MenuResult::None;
  std::optional<size_t> requestLoad;

  std::vector<gsl::not_null<std::unique_ptr<MenuRing>>> rings;
  size_t currentRingIndex = 0;
  bool passOpen = false;
  static bool doOptions(engine::world::World& world, MenuObject& object);
  static void drawMenuObjectDescription(ui::Ui& ui, engine::world::World& world, const MenuObject& object);

  [[nodiscard]] MenuRing& getCurrentRing()
  {
    return *rings.at(currentRingIndex);
  }

  [[nodiscard]] const MenuRing& getCurrentRing() const
  {
    return *rings.at(currentRingIndex);
  }

  void setViewport(const glm::ivec2& viewport);

  [[nodiscard]] const auto& getLightsBuffer() const
  {
    return m_lightsBuffer;
  }

private:
  [[nodiscard]] std::vector<MenuObject> getOptionRingObjects(const engine::world::World& world, bool withHomePolaroid);
  [[nodiscard]] std::vector<MenuObject> getMainRingObjects(const engine::world::World& world);
  [[nodiscard]] std::vector<MenuObject> getKeysRingObjects(const engine::world::World& world);

  ui::Text m_upArrow;
  ui::Text m_downArrow;

  const gsl::not_null<std::shared_ptr<render::scene::Material>> m_material;
  gsl::not_null<std::shared_ptr<render::pass::Framebuffer>> m_fb;

  gsl::not_null<std::shared_ptr<gl::ShaderStorageBuffer<engine::ShaderLight>>> m_lightsBuffer{
    std::make_shared<gl::ShaderStorageBuffer<engine::ShaderLight>>("lights-buffer")};
};
} // namespace menu
