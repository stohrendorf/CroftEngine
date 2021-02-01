#pragma once

#include "menustate.h"

#include <functional>

namespace ui
{
class Label;
}

namespace render
{
class RenderPipeline;

namespace scene
{
class MaterialManager;
}
} // namespace render

namespace menu
{
class RenderSettingsMenuState : public MenuState
{
private:
  static constexpr int16_t LineHeight = 18;
  static constexpr int16_t YPos = -80;
  static constexpr int16_t TotalHeight = 10 * LineHeight + 10;
  static constexpr int16_t YOffset = YPos - TotalHeight;

  std::unique_ptr<MenuState> m_previous;
  size_t m_selected = 0;
  std::vector<std::tuple<std::shared_ptr<ui::Label>, std::function<bool()>, std::function<void()>>> m_labels;
  std::unique_ptr<ui::Label> m_background;
  gsl::not_null<render::RenderPipeline*> m_pipeline;
  gsl::not_null<render::scene::MaterialManager*> m_materialManager;

public:
  static constexpr int16_t PixelWidth = 272;

  explicit RenderSettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                   std::unique_ptr<MenuState> previous,
                                   const gsl::not_null<render::RenderPipeline*>& pipeline,
                                   const gsl::not_null<render::scene::MaterialManager*>& materialManager);
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
};
} // namespace menu
