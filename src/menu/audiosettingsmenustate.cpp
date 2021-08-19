#include "audiosettingsmenustate.h"

#include "core/i18n.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "menudisplay.h"
#include "menuring.h"
#include "savegamelistmenustate.h"
#include "ui/widgets/gridbox.h"
#include "ui/widgets/groupbox.h"
#include "ui/widgets/label.h"
#include "ui/widgets/progressbar.h"

namespace menu
{
AudioSettingsMenuState::AudioSettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                               std::unique_ptr<MenuState> previous,
                                               const MenuDisplay& display)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_grid{std::make_shared<ui::widgets::GridBox>(2 * glm::ivec2{ui::OutlineBorderWidth, ui::OutlineBorderWidth})}
    , m_container{std::make_shared<ui::widgets::GroupBox>(_("Audio Settings"), m_grid)}
    , m_globalVolume{std::make_shared<ui::widgets::ProgressBar>()}
    , m_musicVolume{std::make_shared<ui::widgets::ProgressBar>()}
    , m_sfxVolume{std::make_shared<ui::widgets::ProgressBar>()}
{
  static constexpr int Width = 200;

  m_globalVolume->setSize({Width, ui::FontHeight});
  m_globalVolume->setValue(display.audioSettings.globalVolume);
  m_musicVolume->setSize({Width, ui::FontHeight});
  m_musicVolume->setValue(display.audioSettings.musicVolume);
  m_sfxVolume->setSize({Width, ui::FontHeight});
  m_sfxVolume->setValue(display.audioSettings.sfxVolume);

  m_grid->setExtents(2, 3);
  m_grid->setSelected({0, 0});

  auto label = std::make_shared<ui::widgets::Label>(_("Global"));
  label->fitToContent();
  m_grid->set(0, 0, std::move(label));
  m_grid->set(1, 0, m_globalVolume);

  label = std::make_shared<ui::widgets::Label>(_("Music"));
  label->fitToContent();
  m_grid->set(0, 1, label);
  m_grid->set(1, 1, m_musicVolume);

  label = std::make_shared<ui::widgets::Label>(_("Effects"));
  label->fitToContent();
  m_grid->set(0, 2, label);
  m_grid->set(1, 2, m_sfxVolume);

  m_grid->fitToContent();
  m_container->fitToContent();
}

std::unique_ptr<MenuState>
  AudioSettingsMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  static constexpr float Stepping = 0.1f;

  const auto& inputHandler = world.getEngine().getPresenter().getInputHandler();
  if(inputHandler.hasDebouncedAction(hid::Action::Menu))
    return std::move(m_previous);

  if(inputHandler.hasDebouncedAction(hid::Action::Forward))
  {
    auto [_, y] = m_grid->getSelected();
    if(y > 0)
      m_grid->setSelected({0, y - 1});
  }
  if(inputHandler.hasDebouncedAction(hid::Action::Backward))
  {
    auto [_, y] = m_grid->getSelected();
    if(y < std::get<1>(m_grid->getExtents()) - 1)
      m_grid->setSelected({0, y + 1});
  }

  auto [_, row] = m_grid->getSelected();
  float delta = 0;
  if(inputHandler.hasDebouncedAction(hid::Action::Right))
    delta = Stepping;
  else if(inputHandler.hasDebouncedAction(hid::Action::Left))
    delta = -Stepping;

  std::shared_ptr<ui::widgets::ProgressBar> selected;
  switch(row)
  {
  case 0: selected = m_globalVolume; break;
  case 1: selected = m_musicVolume; break;
  case 2: selected = m_sfxVolume; break;
  }
  Ensures(selected != nullptr);
  selected->setValue(std::clamp(selected->getValue() + delta, 0.0f, 1.0f));

  display.audioSettings.globalVolume = m_globalVolume->getValue();
  display.audioSettings.musicVolume = m_musicVolume->getValue();
  display.audioSettings.sfxVolume = m_sfxVolume->getValue();

  world.getPresenter().getSoundEngine()->getSoLoud().setGlobalVolume(display.audioSettings.globalVolume);

  m_grid->update(true);

  {
    const auto vp = world.getPresenter().getViewport();
    m_container->setPosition({(vp.x - m_container->getSize().x) / 2, vp.y - m_container->getSize().y - 90});
    m_container->draw(ui, world.getPresenter());
  }

  return nullptr;
}
} // namespace menu
