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

  explicit ControlsWidget(const std::string& title,
                          const std::function<std::shared_ptr<ui::widgets::Widget>(hid::Action)>& factory)
      : m_content{std::make_shared<ui::widgets::GridBox>()}
      , m_container{std::make_shared<ui::widgets::GroupBox>(title, m_content)}
  {
    m_content->setExtents(1, 2);

    auto gridBox = std::make_shared<ui::widgets::GridBox>(glm::ivec2{10, ui::OutlineBorderWidth});
    gridBox->setExtents(Columns, 5);
    gridBox->setSelected({1, 0});
    m_controlGroups.emplace_back(gridBox);

    auto groupBox = std::make_shared<ui::widgets::GroupBox>(
      /* translators: TR charmap encoding */ _("Gameplay"), gridBox);
    m_content->set(0, 0, groupBox);

    auto add = [&gridBox, &factory](size_t x0, size_t y, hid::Action action)
    {
      auto label = std::make_shared<ui::widgets::Label>(hid::getName(action));
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

    gridBox = std::make_shared<ui::widgets::GridBox>(glm::ivec2{10, ui::OutlineBorderWidth});
    gridBox->setExtents(Columns, 4);
    m_controlGroups.emplace_back(gridBox);

    groupBox = std::make_shared<ui::widgets::GroupBox>(/* translators: TR charmap encoding */ _("Shortcuts"), gridBox);
    m_content->set(0, 1, groupBox);

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
    return m_container->getPosition();
  }

  [[nodiscard]] glm::ivec2 getSize() const override
  {
    return m_container->getSize();
  }

  void setPosition(const glm::ivec2& position) override
  {
    m_container->setPosition(position);
  }

  void setSize(const glm::ivec2& size) override
  {
    m_container->setSize(size);
  }

  void update(bool hasFocus) override
  {
    m_container->update(hasFocus);
  }

  void fitToContent() override
  {
    for(const auto& gridBox : m_controlGroups)
    {
      gridBox->fitToContent();
      gridBox->setColumnSize(1, gridBox->getColumnSizes()[1] + 2 * ui::FontHeight);
      gridBox->setColumnSize(3, gridBox->getColumnSizes()[3] + 2 * ui::FontHeight);
    }

    for(size_t x = 0; x < Columns; ++x)
    {
      int maxWidth = 0;
      for(const auto& gridBox : m_controlGroups)
      {
        maxWidth = std::max(gridBox->getColumnSizes().at(x), maxWidth);
      }
      for(const auto& gridBox : m_controlGroups)
      {
        gridBox->setColumnSize(x, maxWidth);
      }
    }

    for(size_t x = 0; x < std::get<0>(m_content->getExtents()); ++x)
      for(size_t y = 0; y < std::get<1>(m_content->getExtents()); ++y)
        m_content->getWidget(x, y)->fitToContent();

    m_content->fitToContent();
    m_container->fitToContent();
  }

  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override
  {
    m_container->draw(ui, presenter);
  }

  [[nodiscard]] const auto& getCurrentGridBox() const
  {
    return m_controlGroups.at(std::get<1>(m_content->getSelected()));
  }

  void nextRow()
  {
    do
    {
      const auto& currentGridBox = getCurrentGridBox();
      if(!currentGridBox->nextRow())
      {
        if(!m_content->nextRow())
        {
          // wrap around
          m_content->setSelected({0, 0});
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
        if(!m_content->prevRow())
        {
          // wrap around
          m_content->setSelected({0, m_controlGroups.size() - 1});
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
  std::shared_ptr<ui::widgets::GridBox> m_content;
  std::shared_ptr<ui::widgets::GroupBox> m_container;
  std::vector<std::shared_ptr<ui::widgets::GridBox>> m_controlGroups;
};

ControlsMenuState::ControlsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                     std::unique_ptr<MenuState> previous,
                                     const engine::world::World& world)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
{
  const auto createKeyLabel = [&world](hid::Action action) -> std::shared_ptr<ui::widgets::Widget>
  {
    const auto& actionMap = world.getEngine().getPresenter().getInputHandler().getActionMap();
    auto it = actionMap.find(action);
    if(it == actionMap.end())
    {
      return std::make_shared<ui::widgets::Label>(
        /* translators: TR charmap encoding */ pgettext("ButtonAssignment", "N/A"));
    }
    else
    {
      if(std::holds_alternative<hid::GlfwKey>(it->second))
      {
        return std::make_shared<ui::widgets::Label>(hid::getName(std::get<hid::GlfwKey>(it->second)));
      }
      else
      {
        return std::make_shared<ui::widgets::Label>(hid::getName(std::get<hid::GlfwGamepadButton>(it->second)));
      }
    }
  };
  m_controls = std::make_shared<ControlsWidget>(
    world.getEngine().getPresenter().getInputHandler().getActiveMappingName(), createKeyLabel);
}

std::unique_ptr<MenuState> ControlsMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& /*display*/)
{
  m_controls->fitToContent();

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Backward))
  {
    m_controls->nextRow();
  }
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Forward))
  {
    m_controls->prevRow();
  }
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Right))
  {
    m_controls->nextColumn();
  }
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Left))
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
