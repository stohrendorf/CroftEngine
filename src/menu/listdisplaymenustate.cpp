#include "listdisplaymenustate.h"

#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "menudisplay.h"
#include "ui/label.h"

namespace menu
{
ListDisplayMenuState::ListDisplayMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                           const std::string& heading)
    : SelectedMenuState{ringTransform}
    , m_heading{std::make_unique<ui::Label>(glm::ivec2{0, YOffset - LineHeight - 10}, heading)}
    , m_background{std::make_unique<ui::Label>(glm::ivec2{0, YOffset - LineHeight - 12}, " ")}
{
  m_heading->alignX = ui::Label::Alignment::Center;
  m_heading->alignY = ui::Label::Alignment::Bottom;
  m_heading->addBackground({PixelWidth - 4, 0}, {0, 0});
  m_heading->backgroundGouraud = ui::Label::makeBackgroundCircle(gl::SRGB8{32, 255, 112}, 96, 0);
  m_heading->outline = true;

  m_background->alignX = ui::Label::Alignment::Center;
  m_background->alignY = ui::Label::Alignment::Bottom;
  m_background->addBackground({PixelWidth, LineHeight + TotalHeight + 12}, {0, 0});
  m_background->backgroundGouraud = ui::Label::makeBackgroundCircle(gl::SRGB8{0, 255, 0}, 32, 0);
  m_background->outline = true;
}

std::unique_ptr<MenuState> ListDisplayMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  m_background->draw(ui, world.getPresenter().getTrFont(), world.getPresenter().getViewport());

  const auto page = m_selected / PerPage;
  const auto first = page * PerPage;
  const auto last = std::min(first + PerPage, m_labels.size());
  Expects(first < last);
  for(size_t i = first; i < last; ++i)
  {
    const auto& [lbl, active] = m_labels.at(i);

    auto markActive = [&lbl = *lbl]()
    {
      lbl.addBackground(glm::ivec2{ListDisplayMenuState::PixelWidth - 12, 16}, {0, 0});
      lbl.backgroundGouraud = ui::Label::makeBackgroundCircle(gl::SRGB8{32, 255, 112}, 96, 0);
    };

    if(m_selected == i)
    {
      if(!active)
        lbl->addBackground(glm::ivec2{PixelWidth - 12, 16}, {0, 0});
      lbl->outline = true;
    }
    else
    {
      if(!active)
        lbl->removeBackground();
      lbl->outline = false;
    }

    if(!active)
      lbl->backgroundGouraud.reset();
    else
      markActive();

    lbl->draw(ui, world.getPresenter().getTrFont(), world.getPresenter().getViewport());
  }

  if(!m_heading->text.empty())
    m_heading->draw(ui, world.getPresenter().getTrFont(), world.getPresenter().getViewport());

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
  if(m_selected >= PerPage
     && world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Left))
  {
    m_selected -= PerPage;
  }
  else if(m_selected + PerPage < m_labels.size()
          && world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Right))
  {
    m_selected += PerPage;
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action))
  {
    return onSelected(m_selected, world, display);
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
  {
    return onAborted();
  }

  return nullptr;
}

size_t ListDisplayMenuState::addEntry(const std::string& label)
{
  const auto line = m_labels.size() % PerPage;
  auto lbl = std::make_unique<ui::Label>(glm::ivec2{0, YOffset + line * LineHeight}, label);
  lbl->alignX = ui::Label::Alignment::Center;
  lbl->alignY = ui::Label::Alignment::Bottom;
  m_labels.emplace_back(std::move(lbl), false);
  return m_labels.size() - 1;
}

void ListDisplayMenuState::setActive(size_t idx, bool active)
{
  std::get<1>(m_labels.at(idx)) = active;
}
} // namespace menu
