#include "rendersettingsmenustate.h"

#include "core/i18n.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "menudisplay.h"
#include "render/renderpipeline.h"
#include "ui/widgets/checkbox.h"

#include <gl/glew_init.h>

namespace menu
{
RenderSettingsMenuState::RenderSettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                                 std::unique_ptr<MenuState> previous,
                                                 engine::Engine& engine)
    : ListDisplayMenuState{ringTransform, /* translators: TR charmap encoding */ _("Graphics"), 20, 300, {0, 270}}
    , m_previous{std::move(previous)}
{
  auto addSetting = [this](const std::string& name, std::function<bool()>&& getter, std::function<void()>&& toggler)
  {
    auto checkbox = std::make_shared<ui::widgets::Checkbox>(getListBox().getPosition(), name, getListBox().getSize().x);
    checkbox->setChecked(getter());
    addEntry(checkbox);
    m_checkboxes.emplace_back(std::move(getter), std::move(toggler), std::move(checkbox));
  };

  static const auto toggle = [](engine::Engine& engine, bool& value)
  {
    value = !value;
    engine.applyRenderSettings();
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
  if(gl::hasAnisotropicFilteringExtension())
    addSetting(
      /* translators: TR charmap encoding */ _("Anisotropic Filtering"),
      [&engine]() { return engine.getEngineConfig().renderSettings.anisotropicFiltering; },
      [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.anisotropicFiltering); });
  addSetting(
    /* translators: TR charmap encoding */ _("Water Denoise"),
    [&engine]() { return engine.getEngineConfig().renderSettings.waterDenoise; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.waterDenoise); });
  addSetting(
    /* translators: TR charmap encoding */ _("HBAO"),
    [&engine]() { return engine.getEngineConfig().renderSettings.hbao; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.hbao); });
  addSetting(
    /* translators: TR charmap encoding */ _("High Quality Shadows"),
    [&engine]() { return engine.getEngineConfig().renderSettings.highQualityShadows; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.highQualityShadows); });
  addSetting(
    /* translators: TR charmap encoding */ _("More Lights"),
    [&engine]() { return engine.getEngineConfig().renderSettings.moreLights; },
    [&engine]() { toggle(engine, engine.getEngineConfig().renderSettings.moreLights); });
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
  const auto& [getter, toggler, checkbox] = m_checkboxes.at(idx);
  toggler();
  checkbox->setChecked(getter());
  return nullptr;
}

std::unique_ptr<MenuState> RenderSettingsMenuState::onAborted()
{
  return std::move(m_previous);
}
} // namespace menu
