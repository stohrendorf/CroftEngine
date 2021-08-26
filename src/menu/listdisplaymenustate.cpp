#include "listdisplaymenustate.h"

#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "menudisplay.h"
#include "ui/core.h"
#include "ui/util.h"
#include "ui/widgets/listbox.h"

namespace menu
{
ListDisplayMenuState::ListDisplayMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                           const std::string& heading,
                                           size_t pageSize)
    : SelectedMenuState{ringTransform}
    , m_listBox{std::make_shared<ui::widgets::ListBox>(pageSize)}
    , m_groupBox{heading, m_listBox}
{
}

std::unique_ptr<MenuState> ListDisplayMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  draw(ui, world, display);

  if(world.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(hid::AxisMovement::Forward))
  {
    m_listBox->prevEntry();
  }
  else if(world.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(hid::AxisMovement::Backward))
  {
    m_listBox->nextEntry();
  }
  if(world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Left))
  {
    m_listBox->prevPage();
  }
  else if(world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Right))
  {
    m_listBox->nextPage();
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action))
  {
    return onSelected(m_listBox->getSelected(), world, display);
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
  {
    return onAborted();
  }

  return nullptr;
}

size_t ListDisplayMenuState::append(const std::shared_ptr<ui::widgets::Widget>& widget)
{
  return m_listBox->append(widget);
}

void ListDisplayMenuState::draw(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  m_listBox->fitToContent();
  m_groupBox.fitToContent();

  {
    const auto vp = world.getPresenter().getViewport();
    m_groupBox.setPosition({(vp.x - m_groupBox.getSize().x) / 2, vp.y - m_groupBox.getSize().y - 90});
  }

  m_groupBox.update(true);
  m_groupBox.draw(ui, world.getPresenter());
}
} // namespace menu
