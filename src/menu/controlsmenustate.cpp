#include "controlsmenustate.h"

#include "core/i18n.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/names.h"
#include "menudisplay.h"
#include "ui/widgets/gridbox.h"
#include "ui/widgets/label.h"
#include "ui/widgets/sprite.h"
#include "ui/widgets/vbox.h"

#include <functional>

namespace menu
{
class ControlsMenuState::ControlsWidget final : public ui::widgets::Widget
{
public:
  static constexpr size_t Columns = 6;

  explicit ControlsWidget(const std::function<std::shared_ptr<ui::widgets::Widget>(hid::Action)>& factory)
      : m_stack{std::make_shared<ui::widgets::VBox>(glm::ivec2{0, 0}, glm::ivec2{0, 0})}
  {
    auto gridBox = std::make_shared<ui::widgets::GridBox>(
      glm::ivec2{0, 0}, glm::ivec2{0, 0}, glm::ivec2{10, ui::OutlineBorderWidth});
    gridBox->setExtents(Columns, 5);
    gridBox->setSelected({1, 0});
    m_gridBoxes.emplace_back(gridBox);

    auto groupBox = std::make_shared<ui::widgets::GroupBox>(
      glm::ivec2{0, 0}, glm::ivec2{0, 0}, /* translators: TR charmap encoding */ _("Gameplay"), gridBox);
    m_groupBoxes.emplace_back(groupBox);
    m_stack->append(groupBox);

    auto add = [&gridBox, &factory](size_t x0, size_t y, hid::Action action)
    {
      auto label = std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(action));
      label->fitToContent();
      gridBox->set(x0, y, label);

      auto widget = factory(action);
      widget->fitToContent();
      gridBox->set(x0 + 1, y, widget);
    };

    add(0, 0, hid::Action::Forward);
    add(0, 1, hid::Action::Backward);
    add(0, 2, hid::Action::Left);
    add(0, 3, hid::Action::Right);
    add(0, 4, hid::Action::Jump);

    add(2, 0, hid::Action::StepLeft);
    add(2, 1, hid::Action::StepRight);
    add(2, 2, hid::Action::Walk);
    add(2, 3, hid::Action::Roll);
    add(2, 4, hid::Action::FreeLook);

    add(4, 0, hid::Action::Action);
    add(4, 1, hid::Action::Holster);
    add(4, 2, hid::Action::Menu);

    gridBox = std::make_shared<ui::widgets::GridBox>(
      glm::ivec2{0, 0}, glm::ivec2{0, 0}, glm::ivec2{10, ui::OutlineBorderWidth});
    gridBox->setExtents(Columns, 4);
    m_gridBoxes.emplace_back(gridBox);

    groupBox = std::make_shared<ui::widgets::GroupBox>(
      glm::ivec2{0, 0}, glm::ivec2{0, 0}, /* translators: TR charmap encoding */ _("Shortcuts"), gridBox);
    m_groupBoxes.emplace_back(groupBox);
    m_stack->append(groupBox);

    add(0, 0, hid::Action::DrawPistols);
    add(0, 1, hid::Action::DrawShotgun);
    add(0, 2, hid::Action::DrawUzis);
    add(0, 3, hid::Action::DrawMagnums);

    add(2, 0, hid::Action::ConsumeSmallMedipack);
    add(2, 1, hid::Action::ConsumeLargeMedipack);

    add(4, 0, hid::Action::Save);
    add(4, 1, hid::Action::Load);
    add(4, 2, hid::Action::Screenshot);

    fitToContent();
  }

  [[nodiscard]] glm::ivec2 getPosition() const override
  {
    return m_stack->getPosition();
  }

  [[nodiscard]] glm::ivec2 getSize() const override
  {
    return m_stack->getSize();
  }

  void setPosition(const glm::ivec2& position) override
  {
    m_stack->setPosition(position);
  }

  void setSize(const glm::ivec2& size) override
  {
    m_stack->setSize(size);
  }

  void update(bool hasFocus) override
  {
    m_stack->update(hasFocus);
  }

  void fitToContent() override
  {
    for(const auto& gridBox : m_gridBoxes)
    {
      gridBox->fitToContent();
      gridBox->setColumnSize(1, gridBox->getColumnSizes()[1] + 2 * ui::FontHeight);
      gridBox->setColumnSize(3, gridBox->getColumnSizes()[3] + 2 * ui::FontHeight);
    }

    for(size_t x = 0; x < Columns; ++x)
    {
      int maxWidth = 0;
      for(const auto& gridBox : m_gridBoxes)
      {
        maxWidth = std::max(gridBox->getColumnSizes().at(x), maxWidth);
      }
      for(const auto& gridBox : m_gridBoxes)
      {
        gridBox->setColumnSize(x, maxWidth);
      }
    }

    for(const auto& groupBox : m_groupBoxes)
      groupBox->fitToContent();

    m_stack->fitToContent();
  }

  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override
  {
    m_stack->draw(ui, presenter);
  }

  [[nodiscard]] const auto& getCurrentGridBox() const
  {
    return m_gridBoxes.at(m_stack->getSelected());
  }

  void nextRow()
  {
    do
    {
      const auto& currentGridBox = getCurrentGridBox();
      if(!currentGridBox->nextRow())
      {
        if(!m_stack->nextEntry())
        {
          // wrap around
          m_stack->setSelected(0);
        }

        getCurrentGridBox()->setSelected({std::get<0>(currentGridBox->getSelected()), 0});
      }
    } while(getCurrentGridBox()->getSelectedWidget() == nullptr);
  }

  void prevRow()
  {
    do
    {
      const auto& currentGridBox = getCurrentGridBox();
      if(!currentGridBox->prevRow())
      {
        if(!m_stack->prevEntry())
        {
          // wrap around
          m_stack->setSelected(m_gridBoxes.size() - 1);
        }

        getCurrentGridBox()->setSelected(
          {std::get<0>(currentGridBox->getSelected()), std::get<1>(getCurrentGridBox()->getExtents()) - 1});
      }
    } while(getCurrentGridBox()->getSelectedWidget() == nullptr);
  }

  void nextColumn()
  {
    do
    {
      const auto& currentGridBox = getCurrentGridBox();
      if(!currentGridBox->nextColumn() || !currentGridBox->nextColumn())
      {
        getCurrentGridBox()->setSelected({1, std::get<1>(currentGridBox->getSelected())});
      }
    } while(getCurrentGridBox()->getSelectedWidget() == nullptr);
  }

  void prevColumn()
  {
    do
    {
      const auto& currentGridBox = getCurrentGridBox();
      if(!currentGridBox->prevColumn() || !currentGridBox->prevColumn())
      {
        getCurrentGridBox()->setSelected(
          {std::get<0>(currentGridBox->getExtents()) - 1, std::get<1>(currentGridBox->getSelected())});
      }
    } while(getCurrentGridBox()->getSelectedWidget() == nullptr);
  }

private:
  std::shared_ptr<ui::widgets::VBox> m_stack;
  std::vector<std::shared_ptr<ui::widgets::GridBox>> m_gridBoxes;
  std::vector<std::shared_ptr<ui::widgets::GroupBox>> m_groupBoxes;
};

ControlsMenuState::ControlsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                     std::unique_ptr<MenuState> previous,
                                     const engine::world::World& world)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
{
  const auto createKeyLabel = [&world](hid::Action action) -> std::shared_ptr<ui::widgets::Widget>
  {
    const auto& keyMap = world.getEngine().getPresenter().getInputHandler().getKeyMap();
    auto it = keyMap.find(action);
    if(it == keyMap.end())
      return std::make_shared<ui::widgets::Label>(
        glm::ivec2{0, 0}, /* translators: TR charmap encoding */ pgettext("ButtonAssignment", "N/A"));
    return std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(it->second));
  };
  m_controls = std::make_shared<ControlsWidget>(createKeyLabel);
}

std::unique_ptr<MenuState> ControlsMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& /*display*/)
{
  m_controls->fitToContent();

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Backward))
  {
    m_controls->nextRow();
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Forward))
  {
    m_controls->prevRow();
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Right))
  {
    m_controls->nextColumn();
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Left))
  {
    m_controls->prevColumn();
  }

  const auto vp = world.getPresenter().getViewport();
  m_controls->setPosition({(vp.x - m_controls->getSize().x) / 2, vp.y - 90 - m_controls->getSize().y});
  m_controls->update(true);
  m_controls->draw(ui, world.getPresenter());

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
  {
    return std::move(m_previous);
  }

  return nullptr;
}
} // namespace menu
