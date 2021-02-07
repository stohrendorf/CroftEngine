#include "savegamelistmenustate.h"

#include "engine/engine.h"
#include "engine/i18n.h"
#include "engine/presenter.h"
#include "engine/world.h"
#include "menudisplay.h"
#include "ui/label.h"

#include <boost/format.hpp>

namespace menu
{
SavegameListMenuState::SavegameListMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                             std::unique_ptr<MenuState> previous,
                                             const std::string& heading,
                                             const engine::World& world,
                                             bool loading)
    : MenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_heading{std::make_unique<ui::Label>(glm::ivec2{0, YOffset - LineHeight - 10}, heading)}
    , m_background{std::make_unique<ui::Label>(glm::ivec2{0, YOffset - LineHeight - 12}, " ")}
    , m_loading{loading}
{
  m_heading->alignX = ui::Label::Alignment::Center;
  m_heading->alignY = ui::Label::Alignment::Bottom;
  m_heading->addBackground({PixelWidth - 4, 0}, {0, 0});
  m_heading->backgroundGouraud = ui::Label::makeBackgroundCircle(gl::SRGB8{32, 255, 112}, 128, 0);
  m_heading->outline = true;

  m_background->alignX = ui::Label::Alignment::Center;
  m_background->alignY = ui::Label::Alignment::Bottom;
  m_background->addBackground({PixelWidth, LineHeight + TotalHeight + 12}, {0, 0});
  m_background->backgroundGouraud = ui::Label::makeBackgroundCircle(gl::SRGB8{0, 255, 0}, 96, 32);
  m_background->outline = true;

  const auto savedGames = world.getSavedGames();

  for(size_t i = 0; i < TotalSlots; ++i)
  {
    const auto line = i % PerPage;
    std::string name;
    if(auto it = savedGames.find(i); it != savedGames.end())
    {
      name = it->second.title;
      m_hasSavegame.emplace_back(true);
    }
    else
    {
      name = world.getEngine().i18n(engine::I18n::EmptySlot, i + 1);
      m_hasSavegame.emplace_back(false);
    }
    auto lbl = std::make_unique<ui::Label>(glm::ivec2{0, YOffset + line * LineHeight}, name);
    lbl->alignX = ui::Label::Alignment::Center;
    lbl->alignY = ui::Label::Alignment::Bottom;
    m_labels.emplace_back(std::move(lbl));
  }
}

void SavegameListMenuState::handleObject(engine::World& /*world*/, MenuDisplay& /*display*/, MenuObject& /*object*/)
{
}

std::unique_ptr<MenuState>
  SavegameListMenuState::onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display)
{
  m_background->draw(world.getPresenter().getTrFont(), img, world.getPalette());

  const auto page = m_selected / PerPage;
  const auto first = page * PerPage;
  const auto last = std::min(first + PerPage, m_labels.size());
  Expects(first < last);
  for(size_t i = first; i < last; ++i)
  {
    const auto& lbl = m_labels.at(i);
    if(m_selected == i)
    {
      lbl->addBackground(glm::ivec2{PixelWidth - 12, 16}, glm::ivec2{0});
      lbl->outline = true;
    }
    else
    {
      lbl->removeBackground();
      lbl->outline = false;
    }
    lbl->draw(world.getPresenter().getTrFont(), img, world.getPalette());
  }

  if(!m_heading->text.empty())
    m_heading->draw(world.getPresenter().getTrFont(), img, world.getPalette());

  if(m_selected > 0
     && world.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(hid::AxisMovement::Forward))
  {
    --m_selected;
  }
  else if(m_selected < TotalSlots - 1
          && world.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(
            hid::AxisMovement::Backward))
  {
    ++m_selected;
  }
  if(m_selected >= PerPage
     && world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Left))
  {
    m_selected -= PerPage;
  }
  else if(m_selected < TotalSlots - PerPage
          && world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Right))
  {
    m_selected += PerPage;
  }
  else if(world.getPresenter().getInputHandler().getInputState().action.justChangedTo(true))
  {
    if(!m_loading)
      // TODO confirm overwrite if necessary
      world.save(m_selected);
    else if(m_hasSavegame.at(m_selected))
    {
      display.requestLoad = m_selected;
      display.result = MenuResult::RequestLoad;
    }
    return std::move(m_previous);
  }
  else if(world.getPresenter().getInputHandler().getInputState().menu.justChangedTo(true))
  {
    return std::move(m_previous);
  }

  return nullptr;
}
} // namespace menu
