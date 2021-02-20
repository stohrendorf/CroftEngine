#include "rendersettingsmenustate.h"

#include "engine/engine.h"
#include "engine/i18n.h"
#include "engine/presenter.h"
#include "engine/world.h"
#include "menudisplay.h"
#include "render/renderpipeline.h"

namespace menu
{
namespace
{
void setEnabledBackground(ui::Label& lbl, bool enabled)
{
  if(enabled)
  {
    lbl.addBackground(glm::ivec2{RenderSettingsMenuState::PixelWidth - 12, 16}, {0, 0});
    lbl.backgroundGouraud = ui::Label::makeBackgroundCircle(gl::SRGB8{0, 255, 0}, 255, 32);
  }
  else
  {
    lbl.removeBackground();
  }
}
} // namespace

RenderSettingsMenuState::RenderSettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                                 std::unique_ptr<MenuState> previous,
                                                 engine::Engine& engine)
    : MenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_background{std::make_unique<ui::Label>(glm::ivec2{0, YOffset - 12}, " ")}
{
  m_background->alignX = ui::Label::Alignment::Center;
  m_background->alignY = ui::Label::Alignment::Bottom;
  m_background->addBackground({PixelWidth, TotalHeight + 12}, {0, 0});
  m_background->backgroundGouraud = ui::Label::makeBackgroundCircle(gl::SRGB8{0, 255, 0}, 96, 32);
  m_background->outline = true;

  auto addSetting = [this](const std::string& name, std::function<bool()>&& getter, std::function<void()>&& toggler) {
    auto lbl = std::make_shared<ui::Label>(glm::ivec2{0, YOffset + m_labels.size() * LineHeight}, name);
    lbl->alignX = ui::Label::Alignment::Center;
    lbl->alignY = ui::Label::Alignment::Bottom;
    setEnabledBackground(*lbl, true); // needed to initialize background size for outlining
    setEnabledBackground(*lbl, getter());
    m_labels.emplace_back(lbl, std::move(getter), std::move(toggler));
  };

  static const auto toggle = [](engine::Engine& engine, bool& value) {
    value = !value;
    engine.getPresenter().apply(engine.getEngineConfig().renderSettings);
  };

  addSetting(
    "CRT",
    [&engine]() { return engine.getEngineConfig().renderSettings.crt; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.crt); });
  addSetting(
    "Depth-of-Field",
    [&engine]() { return engine.getEngineConfig().renderSettings.dof; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.dof); });
  addSetting(
    "Lens Distortion",
    [&engine]() { return engine.getEngineConfig().renderSettings.lensDistortion; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.lensDistortion); });
  addSetting(
    "Film Grain",
    [&engine]() { return engine.getEngineConfig().renderSettings.filmGrain; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.filmGrain); });
  addSetting(
    "Fullscreen",
    [&engine]() { return engine.getEngineConfig().renderSettings.fullscreen; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.fullscreen); });
}

void RenderSettingsMenuState::handleObject(engine::World& /*world*/, MenuDisplay& /*display*/, MenuObject& /*object*/)
{
}

std::unique_ptr<MenuState> RenderSettingsMenuState::onFrame(ui::Ui& ui, engine::World& world, MenuDisplay& /*display*/)
{
  m_background->draw(ui, world.getPresenter().getTrFont(), world.getPresenter().getViewport());

  for(size_t i = 0; i < m_labels.size(); ++i)
  {
    const auto& [lbl, getter, toggler] = m_labels.at(i);
    if(m_selected == i)
    {
      lbl->outline = true;
    }
    else
    {
      lbl->outline = false;
    }
    lbl->draw(ui, world.getPresenter().getTrFont(), world.getPresenter().getViewport());
  }

  if(m_selected > 0
     && world.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(hid::AxisMovement::Forward))
  {
    --m_selected;
  }
  else if(m_selected < m_labels.size() - 1
          && world.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(
            hid::AxisMovement::Backward))
  {
    ++m_selected;
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action))
  {
    const auto& [lbl, getter, toggler] = m_labels.at(m_selected);
    toggler();
    setEnabledBackground(*lbl, getter());
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
  {
    return std::move(m_previous);
  }

  return nullptr;
}
} // namespace menu
