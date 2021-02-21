#include "rendersettingsmenustate.h"

#include "engine/engine.h"
#include "engine/i18n.h"
#include "engine/presenter.h"
#include "engine/world.h"
#include "menudisplay.h"
#include "render/renderpipeline.h"

namespace menu
{
RenderSettingsMenuState::RenderSettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                                 std::unique_ptr<MenuState> previous,
                                                 engine::Engine& engine)
    : ListDisplayMenuState{ringTransform, engine.i18n(engine::I18n::Graphics)}
    , m_previous{std::move(previous)}
{
  auto addSetting = [this](const std::string& name, std::function<bool()>&& getter, std::function<void()>&& toggler) {
    setActive(addEntry(name), getter());
    m_handlers.emplace_back(std::move(getter), std::move(toggler));
  };

  static const auto toggle = [](engine::Engine& engine, bool& value) {
    value = !value;
    engine.getPresenter().apply(engine.getEngineConfig().renderSettings);
  };

  addSetting(
    engine.i18n(engine::I18n::CRT),
    [&engine]() { return engine.getEngineConfig().renderSettings.crt; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.crt); });
  addSetting(
    engine.i18n(engine::I18n::DepthOfField),
    [&engine]() { return engine.getEngineConfig().renderSettings.dof; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.dof); });
  addSetting(
    engine.i18n(engine::I18n::LensDistortion),
    [&engine]() { return engine.getEngineConfig().renderSettings.lensDistortion; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.lensDistortion); });
  addSetting(
    engine.i18n(engine::I18n::FilmGrain),
    [&engine]() { return engine.getEngineConfig().renderSettings.filmGrain; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.filmGrain); });
  addSetting(
    engine.i18n(engine::I18n::Fullscreen),
    [&engine]() { return engine.getEngineConfig().renderSettings.fullscreen; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.fullscreen); });
  addSetting(
    engine.i18n(engine::I18n::BilinearFiltering),
    [&engine]() { return engine.getEngineConfig().renderSettings.bilinearFiltering; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.bilinearFiltering); });
}

std::unique_ptr<MenuState>
  RenderSettingsMenuState::onSelected(size_t idx, engine::World& /*world*/, MenuDisplay& /*display*/)
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
