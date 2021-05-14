#include "listdisplaymenustate.h"

#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "menudisplay.h"
#include "ui/label.h"
#include "util.h"

namespace menu
{
ListDisplayMenuState::ListDisplayMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                           const std::string& heading,
                                           size_t pageSize,
                                           int pixelWidth,
                                           int bottomMargin)
    : SelectedMenuState{ringTransform}
    , m_listBox{pageSize, pixelWidth, bottomMargin}
    , m_heading{createHeading(heading,
                              glm::ivec2{0, m_listBox.getTop() - widgets::ListBox::EntryHeight - Padding},
                              {m_listBox.getWidth(), 0})}
    , m_background{
        createFrame({0, m_listBox.getTop() - widgets::ListBox::EntryHeight - Padding - ui::Ui::OutlineBorderWidth},
                    {m_listBox.getWidth() + 2 * ui::Ui::OutlineBorderWidth,
                     widgets::ListBox::EntryHeight + m_listBox.getHeight() + 2 * Padding + ui::Ui::OutlineBorderWidth})}
{
  m_heading->alignX = ui::Label::Alignment::Center;
  m_heading->alignY = ui::Label::Alignment::Bottom;

  m_background->alignX = ui::Label::Alignment::Center;
  m_background->alignY = ui::Label::Alignment::Bottom;
}

std::unique_ptr<MenuState> ListDisplayMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  m_background->draw(ui, world.getPresenter().getTrFont(), world.getPresenter().getViewport());
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
} // namespace menu
