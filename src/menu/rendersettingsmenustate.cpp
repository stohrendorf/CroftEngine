#include "rendersettingsmenustate.h"

#include "core/i18n.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "menudisplay.h"
#include "render/renderpipeline.h"

namespace menu
{
RenderSettingsMenuState::RenderSettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                                 std::unique_ptr<MenuState> previous,
                                                 engine::Engine& engine)
    : ListDisplayMenuState{ringTransform, /* translators: TR charmap encoding */ _("Graphics")}
    , m_previous{std::move(previous)}
{
  auto addSetting = [this](const std::string& name, std::function<bool()>&& getter, std::function<void()>&& toggler)
  {
    setActive(addEntry(name), getter());
    m_handlers.emplace_back(std::move(getter), std::move(toggler));
  };

  static const auto toggle = [](engine::Engine& engine, bool& value)
  {
    value = !value;
    engine.getPresenter().apply(engine.getEngineConfig().renderSettings);
  };

  addSetting(
    /* translators: TR charmap encoding */ _("CRT"),
    [&engine]() { return engine.getEngineConfig().renderSettings.crt; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.crt); });
  addSetting(
    /* translators: TR charmap encoding */ _("Depth-of-Field"),
    [&engine]() { return engine.getEngineConfig().renderSettings.dof; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.dof); });
  addSetting(
    /* translators: TR charmap encoding */ _("Lens Distortion"),
    [&engine]() { return engine.getEngineConfig().renderSettings.lensDistortion; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.lensDistortion); });
  addSetting(
    /* translators: TR charmap encoding */ _("Film Grain"),
    [&engine]() { return engine.getEngineConfig().renderSettings.filmGrain; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.filmGrain); });
  addSetting(
    /* translators: TR charmap encoding */ _("Fullscreen"),
    [&engine]() { return engine.getEngineConfig().renderSettings.fullscreen; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.fullscreen); });
  addSetting(
    /* translators: TR charmap encoding */ _("Bilinear Filtering"),
    [&engine]() { return engine.getEngineConfig().renderSettings.bilinearFiltering; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.bilinearFiltering); });
  addSetting(
    /* translators: TR charmap encoding */ _("Water Denoise"),
    [&engine]() { return engine.getEngineConfig().renderSettings.waterDenoise; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.waterDenoise); });
  addSetting(
    /* translators: TR charmap encoding */ _("Performance Meter"),
    [&engine]() { return engine.getEngineConfig().displaySettings.performanceMeter; },
    [&engine]()
    {
      auto& b = engine.getEngineConfig().displaySettings.performanceMeter;
      b = !b;
    });
}

std::unique_ptr<MenuState>
  RenderSettingsMenuState::onSelected(size_t idx, engine::world::World& /*world*/, MenuDisplay& /*display*/)
{
  const auto& [getter, toggler] = m_handlers.at(idx);
  toggler();
  setActive(idx, getter());
  return nullptr;
}

std::unique_ptr<MenuState> RenderSettingsMenuState::onAborted()
{
  return std::move(m_previous);
}
} // namespace menu
