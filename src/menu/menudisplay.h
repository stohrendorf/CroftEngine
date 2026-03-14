#pragma once

#include "engine/items_tr1.h"
#include "menuresult.h"
#include "menuringtransform.h"
#include "ui/text.h"

#include <cstddef>
#include <cstdint>
#include <gl/buffer.h>
#include <glm/vec2.hpp>
#include <gsl-lite/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <optional>
#include <vector>

namespace engine::world
{
class World;
}

namespace engine
{
struct ShaderLight;
}

namespace ui
{
class Ui;
}

namespace render::material
{
class Material;
}

namespace render::pass
{
class Framebuffer;
}

namespace menu
{
enum class InventoryMode : uint8_t
{
  /// adds "items" and "inventory" rings
  GameMode,
  /// disallows menu closing, adds home polaroid
  TitleMode,
  /// disallows menu closing, uses "game over" label instead of "option" for menu
  DeathMode
};

struct MenuObject;
struct MenuRing;
class MenuState;

enum class SaveGamePageMode : uint8_t
{
  Skip,
  NewGame,
  Save,
  Restart
};

struct MenuDisplay
{
  explicit MenuDisplay(InventoryMode mode,
                       SaveGamePageMode saveGamePageMode,
                       bool allowPassportExit,
                       engine::world::World& world,
                       const glm::ivec2& viewport);
  ~MenuDisplay();

  InventoryMode mode;
  std::optional<engine::TR1ItemId> inventoryChosen;
  bool allowMenuClose = true;
  SaveGamePageMode saveGamePageMode;
  bool allowPassportExit;

  std::shared_ptr<MenuRingTransform> ringTransform = std::make_shared<MenuRingTransform>();
  std::unique_ptr<MenuState> m_currentState;

  void renderObjectsToFramebuffer(engine::world::World& world);
  void tick(engine::world::World& world);
  void constructUi(ui::Ui& ui, engine::world::World& world);
  void constructNavigationHints(ui::Ui& ui, const engine::world::World& world) const;
  void renderFramebuffer(const engine::world::World& world);
  MenuResult result = MenuResult::None;
  std::optional<size_t> requestLoad;
  size_t requestLevelSequenceIndex = 0;

  std::vector<gslu::nn_unique<MenuRing>> rings;
  size_t currentRingIndex = 0;
  bool passOpen = false;
  static bool doOptions(engine::world::World& world, MenuObject& object);
  static void constructMenuObjectDescription(ui::Ui& ui, engine::world::World& world, const MenuObject& object);

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
  [[nodiscard]] std::vector<MenuObject> getOptionRingObjects(engine::world::World& world, bool withHomePolaroid) const;
  [[nodiscard]] std::vector<MenuObject> getMainRingObjects(engine::world::World& world) const;
  [[nodiscard]] std::vector<MenuObject> getKeysRingObjects(engine::world::World& world) const;

  ui::Text m_upArrow;
  ui::Text m_downArrow;

  gslu::nn_shared<render::material::Material> m_material;
  gslu::nn_shared<render::pass::Framebuffer> m_fb;

  gslu::nn_shared<gl::ShaderStorageBuffer<engine::ShaderLight>> m_lightsBuffer;
};
} // namespace menu