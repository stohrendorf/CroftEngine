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

#include <functional>

namespace menu
{
class ControlsMenuState::ControlsWidget final : public ui::widgets::Widget
{
public:
  static constexpr size_t Columns = 6;

  explicit ControlsWidget(const std::function<std::shared_ptr<ui::widgets::Widget>(hid::Action)>& factory,
                          const std::string& title)
      : m_gridBox{std::make_shared<ui::widgets::GridBox>(
        glm::ivec2{0, 0}, glm::ivec2{0, 0}, glm::ivec2{10, ui::OutlineBorderWidth})}
      , m_groupBox{std::make_shared<ui::widgets::GroupBox>(glm::ivec2{0, 0}, glm::ivec2{0, 0}, title, m_gridBox)}
  {
    auto add = [this, &factory](size_t x0, size_t y, hid::Action action)
    {
      auto label = std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, hid::getName(action));
      label->fitToContent();
      m_gridBox->set(x0, y, label);

      auto widget = factory(action);
      widget->fitToContent();
      m_gridBox->set(x0 + 1, y, widget);
    };

    m_gridBox->setExtents(Columns, 8);

    add(0, 0, hid::Action::Forward);
    add(0, 1, hid::Action::Backward);
    add(0, 2, hid::Action::Left);
    add(0, 3, hid::Action::Right);

    add(2, 0, hid::Action::StepLeft);
    add(2, 1, hid::Action::StepRight);
    add(2, 2, hid::Action::MoveSlow);
    add(2, 3, hid::Action::Roll);

    add(4, 0, hid::Action::Action);
    add(4, 1, hid::Action::Holster);
    add(4, 2, hid::Action::Menu);
    add(4, 3, hid::Action::FreeLook);

    add(0, 4, hid::Action::DrawPistols);
    add(0, 5, hid::Action::DrawShotgun);
    add(0, 6, hid::Action::DrawUzis);
    add(0, 7, hid::Action::DrawMagnums);

    add(2, 4, hid::Action::ConsumeSmallMedipack);
    add(2, 5, hid::Action::ConsumeLargeMedipack);

    add(4, 4, hid::Action::Save);
    add(4, 5, hid::Action::Load);
    add(4, 6, hid::Action::Screenshot);

    m_gridBox->setSelected({1, 0});

    fitToContent();
  }

  [[nodiscard]] glm::ivec2 getPosition() const override
  {
    return m_groupBox->getPosition();
  }

  [[nodiscard]] glm::ivec2 getSize() const override
  {
    return m_groupBox->getSize();
  }

  void setPosition(const glm::ivec2& position) override
  {
    m_groupBox->setPosition(position);
  }

  void setSize(const glm::ivec2& size) override
  {
    m_groupBox->setSize(size);
  }

  void update(bool hasFocus) override
  {
    m_groupBox->update(hasFocus);
  }

  void fitToContent() override
  {
    m_gridBox->fitToContent();
    m_gridBox->setRowSize(3, 2 * ui::FontHeight);
    m_gridBox->setColumnSize(1, m_gridBox->getColumnSizes()[1] + 2 * ui::FontHeight);
    m_gridBox->setColumnSize(3, m_gridBox->getColumnSizes()[3] + 2 * ui::FontHeight);
    m_groupBox->fitToContent();
  }

  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override
  {
    m_groupBox->draw(ui, presenter);
  }

  [[nodiscard]] auto getSelectedColumn() const
  {
    return std::get<0>(m_gridBox->getSelected());
  }

  [[nodiscard]] auto getSelectedRow() const
  {
    return std::get<1>(m_gridBox->getSelected());
  }

  auto nextRow()
  {
    return m_gridBox->nextRow();
  }

  auto prevRow()
  {
    return m_gridBox->prevRow();
  }

  auto nextColumn()
  {
    return m_gridBox->nextColumn();
  }

  auto prevColumn()
  {
    return m_gridBox->prevColumn();
  }

  void selectFirstRow(size_t column)
  {
    m_gridBox->setSelected({column, 0});
  }

  void selectLastRow(size_t column)
  {
    m_gridBox->setSelected({column, std::get<1>(m_gridBox->getExtents()) - 1});
  }

  void selectFirstColumn()
  {
    m_gridBox->setSelected({1, getSelectedRow()});
  }

  void selectLastColumn()
  {
    m_gridBox->setSelected({std::get<0>(m_gridBox->getExtents()) - 1, getSelectedRow()});
  }

  [[nodiscard]] auto getColumnSize(size_t column) const
  {
    return m_gridBox->getColumnSizes().at(column);
  }

  void setColumnSize(size_t column, int size)
  {
    m_gridBox->setColumnSize(column, size);
    m_groupBox->fitToContent();
  }

  [[nodiscard]] bool selectionIsValid() const
  {
    return m_gridBox->getSelectedWidget() != nullptr;
  }

private:
  std::shared_ptr<ui::widgets::GridBox> m_gridBox;
  std::shared_ptr<ui::widgets::GroupBox> m_groupBox;
};

ControlsMenuState::ControlsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                     std::unique_ptr<MenuState> previous,
                                     const engine::world::World& world)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_allControls{std::make_shared<ui::widgets::GridBox>(glm::ivec2{0, 0}, glm::ivec2{0, 0})}
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

  const auto& layout = world.getControllerLayouts().at("PS");
  const auto createButtonLabel = [&world, &layout](hid::Action action) -> std::shared_ptr<ui::widgets::Widget>
  {
    const auto& buttonMap = world.getEngine().getPresenter().getInputHandler().getGamepadMap();
    auto it = buttonMap.find(action);
    if(it == buttonMap.end())
      return std::make_shared<ui::widgets::Label>(
        glm::ivec2{0, 0}, /* translators: TR charmap encoding */ pgettext("ButtonAssignment", "N/A"));
    return std::make_shared<ui::widgets::Sprite>(glm::ivec2{0, 0}, glm::ivec2{0, 0}, layout.at(it->second));
  };
  m_controls.emplace_back(
    std::make_shared<ControlsWidget>(createKeyLabel, /* translators: TR charmap encoding */ _("Keyboard")));

  m_controls.emplace_back(
    std::make_shared<ControlsWidget>(createButtonLabel, /* translators: TR charmap encoding */ _("Gamepad")));

  m_allControls->setExtents(1, 2);
  m_allControls->set(0, 0, m_controls[0]);
  m_allControls->set(0, 1, m_controls[1]);

  m_allControls->fitToContent();

  // align columns
  for(size_t x = 0; x < ControlsWidget::Columns; ++x)
  {
    int maxColumnWidth = 0;
    for(const auto& controlBox : m_controls)
    {
      maxColumnWidth = std::max(maxColumnWidth, controlBox->getColumnSize(x));
    }
    for(const auto& controlBox : m_controls)
    {
      controlBox->setColumnSize(x, maxColumnWidth);
    }
  }
}

std::unique_ptr<MenuState> ControlsMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& /*display*/)
{
  {
    auto getCurrentControls = [this]() -> std::shared_ptr<ControlsWidget>& {
      return m_controls.at(std::get<1>(m_allControls->getSelected()));
    };

    if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Backward))
    {
      do
      {
        const auto& currentControls = getCurrentControls();
        if(!currentControls->nextRow())
        {
          if(!m_allControls->nextRow())
          {
            // wrap around
            m_allControls->setSelected({std::get<0>(m_allControls->getSelected()), 0});
          }

          const auto& nextControls = m_controls.at(std::get<1>(m_allControls->getSelected()));
          nextControls->selectFirstRow(currentControls->getSelectedColumn());
        }
      } while(!getCurrentControls()->selectionIsValid());
    }
    else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Forward))
    {
      do
      {
        const auto& currentControls = getCurrentControls();
        if(!currentControls->prevRow())
        {
          if(!m_allControls->prevRow())
          {
            // wrap around
            m_allControls->setSelected(
              {std::get<0>(m_allControls->getSelected()), std::get<1>(m_allControls->getExtents()) - 1});
          }

          const auto& nextControls = m_controls.at(std::get<1>(m_allControls->getSelected()));
          nextControls->selectLastRow(currentControls->getSelectedColumn());
        }
      } while(!getCurrentControls()->selectionIsValid());
    }
    else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Right))
    {
      do
      {
        const auto& currentControls = getCurrentControls();
        if(!currentControls->nextColumn() || !currentControls->nextColumn())
        {
          currentControls->selectFirstColumn();
        }
      } while(!getCurrentControls()->selectionIsValid());
    }
    else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Left))
    {
      do
      {
        const auto& currentControls = getCurrentControls();
        if(!currentControls->prevColumn() || !currentControls->prevColumn())
        {
          currentControls->selectLastColumn();
        }
      } while(!getCurrentControls()->selectionIsValid());
    }
  }

  const auto vp = world.getPresenter().getViewport();
  m_allControls->setPosition({(vp.x - m_allControls->getSize().x) / 2, vp.y - 90 - m_allControls->getSize().y});
  m_allControls->update(true);
  m_allControls->draw(ui, world.getPresenter());

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
  {
    return std::move(m_previous);
  }

  return nullptr;
}
} // namespace menu
