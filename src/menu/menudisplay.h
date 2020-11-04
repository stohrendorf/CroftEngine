#pragma once

#include "menuringtransform.h"
#include "ui/label.h"

namespace menu
{
enum class InventoryMode
{
  GameMode,
  TitleMode,
  KeysMode,
  SaveMode,
  LoadMode,
  DeathMode
};

struct MenuObject;
struct MenuRing;
struct MenuDisplay;

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

  virtual void begin()
  {
  }
  virtual void handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object) = 0;
  virtual std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::Engine& engine, MenuDisplay& display)
    = 0;
};

class ResetItemTransformMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 16_frame;
  core::Frame m_duration{Duration};
  std::unique_ptr<MenuState> m_next;

public:
  explicit ResetItemTransformMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                       std::unique_ptr<MenuState>&& next)
      : MenuState{ringTransform}
      , m_next{std::move(next)}
  {
  }

  void handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::Engine& engine, MenuDisplay& display) override;
};

class ApplyItemTransformMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 16_frame;
  core::Frame m_duration{0_frame};

public:
  explicit ApplyItemTransformMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform)
      : MenuState{ringTransform}
  {
  }

  void handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::Engine& engine, MenuDisplay& display) override;
};

class DoneMenuState : public MenuState
{
public:
  explicit DoneMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform)
      : MenuState{ringTransform}
  {
  }

  void handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::Engine& engine, MenuDisplay& display) override;
};

class DeflateRingMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 32_frame;
  core::Frame m_duration{Duration};
  std::unique_ptr<MenuState> m_next;
  core::Length m_initialRadius{};
  core::Length m_cameraSpeedY{};

public:
  explicit DeflateRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                std::unique_ptr<MenuState> next);

  void begin() override
  {
    // TODO fadeOutInventory(mode != InventoryMode::TitleMode);
    m_initialRadius = m_ringTransform->radius;
    m_cameraSpeedY = (-256_len - m_ringTransform->cameraPos.Y) / Duration * 1_frame;
  }

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::Engine& engine, MenuDisplay& display) override;
  void handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object) override;
};

class FinishItemAnimationMenuState : public MenuState
{
private:
  std::unique_ptr<MenuState> m_next;

public:
  explicit FinishItemAnimationMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                        std::unique_ptr<MenuState> next)
      : MenuState{ringTransform}
      , m_next{std::move(next)}
  {
  }

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::Engine& engine, MenuDisplay& display) override;
  void handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object) override;
};

class DeselectingMenuState : public MenuState
{
public:
  explicit DeselectingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, engine::Engine& engine);

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::Engine& engine, MenuDisplay& display) override;
  void handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object) override;
};

class IdleRingMenuState : public MenuState
{
private:
  bool m_autoSelect;

public:
  explicit IdleRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, bool autoSelect)
      : MenuState{ringTransform}
      , m_autoSelect{autoSelect}
  {
  }

  void handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::Engine& engine, MenuDisplay& display) override;
};

class SwitchRingMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 24_frame;

  core::Frame m_duration{Duration};
  core::Length m_radiusSpeed{};
  core::Angle m_targetCameraRotX{};
  std::shared_ptr<MenuRing> m_next;
  const bool m_down;

public:
  explicit SwitchRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                               std::shared_ptr<MenuRing> next,
                               bool down);

  void begin() override
  {
    m_radiusSpeed = m_ringTransform->radius / Duration * 1_frame;
    m_targetCameraRotX = m_down ? -45_deg : 45_deg;
  }

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::Engine& engine, MenuDisplay& display) override;
  void handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object) override;
};

class SelectedMenuState : public MenuState
{
public:
  explicit SelectedMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform)
      : MenuState{ringTransform}
  {
  }

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::Engine& engine, MenuDisplay& display) override;
  void handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object) override;
};

class InflateRingMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 24_frame;
  core::Frame m_duration{Duration};
  core::Angle m_initialCameraRotX{};
  core::Length m_radiusSpeed{};
  core::Length m_cameraSpeedY{};

public:
  explicit InflateRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform);

  void begin() override
  {
    m_initialCameraRotX = m_ringTransform->cameraRotX;
    m_radiusSpeed = (688_len - m_ringTransform->radius) / Duration * 1_frame;
    m_cameraSpeedY = (-256_len - m_ringTransform->cameraPos.Y) / Duration * 1_frame;
  }
  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::Engine& engine, MenuDisplay& display) override;
  void handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object) override;
};

class RotateLeftRightMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 24_frame;
  size_t m_targetObject{0};
  core::Frame m_duration{Duration};
  QS_COMBINE_UNITS(core::Angle, /, core::Frame) m_rotSpeed;
  std::unique_ptr<MenuState> m_prev;

public:
  explicit RotateLeftRightMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                    bool left,
                                    const MenuRing& ring,
                                    std::unique_ptr<MenuState>&& prev);

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::Engine& engine, MenuDisplay& display) override;
  void handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object) override;
};

struct MenuDisplay
{
  InventoryMode mode = InventoryMode::GameMode;
  std::array<std::shared_ptr<ui::Label>, 7> objectTexts;
  std::shared_ptr<ui::Label> ammoText;
  std::optional<engine::TR1ItemId> inventoryChosen{};
  int musicVolume = 10;
  int passportPage;
  int selectedOption;

  std::unique_ptr<MenuState> m_currentState;

  bool init(engine::Engine& engine);
  void display(gl::Image<gl::SRGBA8>& img, engine::Engine& engine);
  void finalize(engine::Engine& engine);
  bool isDone = false;

  std::shared_ptr<MenuRing> keysRing;
  std::shared_ptr<MenuRing> optionsRing;
  std::shared_ptr<MenuRing> mainRing;
  std::shared_ptr<MenuRing> currentRing;
  std::shared_ptr<MenuRingTransform> ringTransform = std::make_shared<MenuRingTransform>();
  bool passOpen = false;
  bool doOptions(gl::Image<gl::SRGBA8>& img, engine::Engine& engine, MenuObject& object);
  void updateMenuObjectDescription(engine::Engine& engine, const MenuObject& object);
  void clearMenuObjectDescription();
  void updateRingTitle(const MenuRing& ring, const engine::Engine& engine);

private:
  [[nodiscard]] static std::vector<MenuObject> getOptionRingObjects(bool withHomePolaroid);
  [[nodiscard]] static std::vector<MenuObject> getMainRingObjects(const engine::Engine& engine);
  [[nodiscard]] static std::vector<MenuObject> getKeysRingObjects(const engine::Engine& engine);

  void clearRingTitle();
};
} // namespace menu
