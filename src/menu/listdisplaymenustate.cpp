#include "listdisplaymenustate.h"

#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "menudisplay.h"
#include "ui/core.h"
#include "util.h"

namespace menu
{
ListDisplayMenuState::ListDisplayMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                           const std::string& heading,
                                           size_t pageSize,
                                           int width,
                                           const glm::ivec2& position)
    : SelectedMenuState{ringTransform}
    , m_position{position}
    , m_listBox{pageSize, width, {0, 0}}
    , m_heading{createHeading(heading, {0, 0}, {m_listBox.getSize().x, 0})}
    , m_background{createFrame({0, 0}, {0, 0})}
{
  setPosition(m_position);
}

std::unique_ptr<MenuState> ListDisplayMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  {
    const auto vp = world.getPresenter().getViewport();
    setPosition({(vp.x - m_listBox.getSize().x) / 2, vp.y - m_listBox.getSize().y - 90});
  }

  m_background->bgndSize = {m_listBox.getSize().x + 2 * ui::OutlineBorderWidth,
                            ui::FontHeight + m_listBox.getSize().y + 2 * (Padding + ui::OutlineBorderWidth)};
  m_background->draw(ui, world.getPresenter().getTrFont(), world.getPresenter().getViewport());
  m_listBox.update(true);
  m_listBox.draw(ui, world.getPresenter());

  if(!m_heading->text.empty())
    m_heading->draw(ui, world.getPresenter().getTrFont(), world.getPresenter().getViewport());

  if(world.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(hid::AxisMovement::Forward))
  {
    m_listBox.prevEntry();
  }
  else if(world.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(hid::AxisMovement::Backward))
  {
    m_listBox.nextEntry();
  }
  if(world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Left))
  {
    m_listBox.prevPage();
  }
  else if(world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Right))
  {
    m_listBox.nextPage();
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action))
  {
    return onSelected(m_listBox.getSelected(), world, display);
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
  {
    return onAborted();
  }

  return nullptr;
}

void ListDisplayMenuState::setPosition(const glm::ivec2& position)
{
  m_position = position;
  m_background->pos = m_position;
  m_heading->pos = m_position + glm::ivec2{ui::OutlineBorderWidth, ui::OutlineBorderWidth};
  m_listBox.setPosition(m_position
                        + glm::ivec2{ui::OutlineBorderWidth, ui::FontHeight + Padding + ui::OutlineBorderWidth});
}

size_t ListDisplayMenuState::addEntry(const std::shared_ptr<widgets::Widget>& widget)
{
  return m_listBox.addEntry(widget);
}
} // namespace menu
