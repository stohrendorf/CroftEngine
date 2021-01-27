#pragma once

#include "engine/items_tr1.h"
#include "menuringtransform.h"

#include <gl/image.h>

namespace ui
{
struct Label;
}

namespace menu
{
struct MenuDisplay;
struct MenuObject;
struct MenuRing;
enum class InventoryMode;
enum class MenuResult;

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
  virtual void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) = 0;
  virtual std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display)
    = 0;

  template<typename T, typename... Ts>
  auto create(Ts&&... args) -> std::enable_if_t<std::is_base_of_v<MenuState, T>, std::unique_ptr<T>>
  {
    return std::make_unique<T>(m_ringTransform, std::forward<Ts>(args)...);
  }
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

  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
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

  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
};

class DoneMenuState : public MenuState
{
public:
  explicit DoneMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, MenuResult result)
      : MenuState{ringTransform}
      , m_result{result}
  {
  }

  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;

private:
  const MenuResult m_result;
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

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
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

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
};

class SetItemTypeMenuState : public MenuState
{
private:
  const engine::TR1ItemId m_type;
  std::unique_ptr<MenuState> m_next;

public:
  explicit SetItemTypeMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                const engine::TR1ItemId type,
                                std::unique_ptr<MenuState> next)
      : MenuState{ringTransform}
      , m_type{type}
      , m_next{std::move(next)}
  {
  }

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
};

class DeselectingMenuState : public MenuState
{
public:
  explicit DeselectingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, engine::World& world);

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
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

  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
};

class SwitchRingMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 24_frame;

  core::Frame m_duration{Duration};
  core::Length m_radiusSpeed{};
  core::Angle m_targetCameraRotX{};
  size_t m_next;
  const bool m_down;

public:
  explicit SwitchRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, size_t next, bool down);

  void begin() override
  {
    m_radiusSpeed = m_ringTransform->radius / Duration * 1_frame;
    m_targetCameraRotX = m_down ? -45_deg : 45_deg;
  }

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
};

class SelectedMenuState : public MenuState
{
public:
  explicit SelectedMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform)
      : MenuState{ringTransform}
  {
  }

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
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
  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
};

class RotateLeftRightMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 24_frame;
  size_t m_targetObject{0};
  core::Frame m_duration{Duration};
  // cppcheck-suppress syntaxError
  QS_COMBINE_UNITS(core::Angle, /, core::Frame) m_rotSpeed;
  std::unique_ptr<MenuState> m_prev;

public:
  explicit RotateLeftRightMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                    bool left,
                                    const MenuRing& ring,
                                    std::unique_ptr<MenuState>&& prev);

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
};

class PassportMenuState : public MenuState
{
private:
  static constexpr int LoadGamePage = 0;
  static constexpr int SaveGamePage = 1;
  static constexpr int ExitGamePage = 2;
  static constexpr core::Frame FramesPerPage = 5_frame;

  const bool m_allowExit;
  const bool m_allowSave;
  std::optional<int> m_forcePage;
  std::unique_ptr<ui::Label> m_passportText;

  std::unique_ptr<MenuState> close(MenuDisplay& display, int page, MenuObject& passport);

public:
  explicit PassportMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, InventoryMode mode);

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
};

class SavegameListMenuState : public MenuState
{
private:
  static constexpr int16_t PixelWidth = 272;
  static constexpr int16_t LineHeight = 18;
  static constexpr int16_t YPos = -80;
  static constexpr int16_t PerPage = 10;
  static constexpr int16_t TotalHeight = PerPage * LineHeight + 10;
  static constexpr int16_t YOffset = YPos - TotalHeight;
  static constexpr size_t TotalSlots = 20;

  std::unique_ptr<MenuState> m_previous;
  size_t m_selected = 0;
  std::vector<std::unique_ptr<ui::Label>> m_labels;
  std::unique_ptr<ui::Label> m_heading;
  std::unique_ptr<ui::Label> m_background;

public:
  explicit SavegameListMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                 std::unique_ptr<MenuState> previous,
                                 const std::string& heading);
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
};
} // namespace menu
