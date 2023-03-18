#include "audiosettingsmenustate.h"

#include "audio/soundengine.h"
#include "core/i18n.h"
#include "engine/audioengine.h"
#include "engine/audiosettings.h"
#include "engine/engine.h"
#include "engine/engineconfig.h"
#include "engine/presenter.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "menu/selectedmenustate.h"
#include "ui/core.h"
#include "ui/ui.h"
#include "ui/widgets/gridbox.h"
#include "ui/widgets/groupbox.h"
#include "ui/widgets/label.h"
#include "ui/widgets/progressbar.h"

#include <algorithm>
#include <boost/throw_exception.hpp>
#include <cstddef>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace menu
{
AudioSettingsMenuState::AudioSettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                               std::unique_ptr<MenuState> previous)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_grid{std::make_shared<ui::widgets::GridBox>(2 * glm::ivec2{ui::OutlineBorderWidth, ui::OutlineBorderWidth})}
    , m_container{std::make_shared<ui::widgets::GroupBox>(/* translators: TR charmap encoding */ _("Audio Settings"),
                                                          m_grid)}
    , m_globalVolume{std::make_shared<ui::widgets::ProgressBar>()}
    , m_musicVolume{std::make_shared<ui::widgets::ProgressBar>()}
    , m_sfxVolume{std::make_shared<ui::widgets::ProgressBar>()}
{
  static constexpr int Width = 200;

  m_globalVolume->setSize({Width, ui::FontHeight});
  m_musicVolume->setSize({Width, ui::FontHeight});
  m_sfxVolume->setSize({Width, ui::FontHeight});

  m_grid->setExtents(2, 3);
  m_grid->setSelected({0, 0});

  auto label = std::make_shared<ui::widgets::Label>(/* translators: TR charmap encoding */ _("Global"));
  label->fitToContent();
  m_grid->set(0, 0, std::move(label));
  m_grid->set(1, 0, m_globalVolume);

  label = std::make_shared<ui::widgets::Label>(/* translators: TR charmap encoding */ _("Music"));
  label->fitToContent();
  m_grid->set(0, 1, label);
  m_grid->set(1, 1, m_musicVolume);

  label = std::make_shared<ui::widgets::Label>(/* translators: TR charmap encoding */ _("Effects"));
  label->fitToContent();
  m_grid->set(0, 2, label);
  m_grid->set(1, 2, m_sfxVolume);

  m_grid->fitToContent();
  m_container->fitToContent();
}

std::unique_ptr<MenuState>
  AudioSettingsMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& /*display*/)
{
  static constexpr float Stepping = 0.1f;
  auto& audioSettings = world.getEngine().getEngineConfig()->audioSettings;
  m_globalVolume->setValue(audioSettings.globalVolume);
  m_musicVolume->setValue(audioSettings.musicVolume);
  m_sfxVolume->setValue(audioSettings.sfxVolume);

  const auto& inputHandler = world.getEngine().getPresenter().getInputHandler();
  if(inputHandler.hasDebouncedAction(hid::Action::Return))
    return std::move(m_previous);

  if(inputHandler.hasDebouncedAction(hid::Action::MenuUp))
  {
    auto [_, y] = m_grid->getSelected();
    if(y > 0)
      m_grid->setSelected({0, y - 1});
  }
  if(inputHandler.hasDebouncedAction(hid::Action::MenuDown))
  {
    auto [_, y] = m_grid->getSelected();
    if(y < std::get<1>(m_grid->getExtents()) - 1)
      m_grid->setSelected({0, y + 1});
  }

  auto [_, row] = m_grid->getSelected();
  float delta = 0;
  if(inputHandler.hasDebouncedAction(hid::Action::MenuRight))
    delta = Stepping;
  else if(inputHandler.hasDebouncedAction(hid::Action::MenuLeft))
    delta = -Stepping;

  std::shared_ptr<ui::widgets::ProgressBar> selected;
  switch(row)
  {
  case 0:
    selected = m_globalVolume;
    break;
  case 1:
    selected = m_musicVolume;
    break;
  case 2:
    selected = m_sfxVolume;
    break;
  default:
    BOOST_THROW_EXCEPTION(std::runtime_error("invalid volume type selection"));
  }
  gsl_Assert(selected != nullptr);
  selected->setValue(std::clamp(selected->getValue() + delta, 0.0f, 1.0f));

  audioSettings.globalVolume = m_globalVolume->getValue();
  audioSettings.musicVolume = m_musicVolume->getValue();
  audioSettings.sfxVolume = m_sfxVolume->getValue();

  world.getPresenter().getSoundEngine()->setListenerGain(audioSettings.globalVolume);
  world.getAudioEngine().setMusicGain(audioSettings.musicVolume);
  world.getAudioEngine().setSfxGain(audioSettings.sfxVolume);

  if(delta != 0)
  {
    world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuGamePageTurn, nullptr);
  }

  m_grid->update(true);

  m_container->setPosition(
    {(ui.getSize().x - m_container->getSize().x) / 2, ui.getSize().y - m_container->getSize().y - 90});
  m_container->draw(ui, world.getPresenter());

  return nullptr;
}
} // namespace menu
