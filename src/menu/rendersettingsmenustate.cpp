#include "rendersettingsmenustate.h"

#include "core/i18n.h"
#include "engine/displaysettings.h"
#include "engine/engine.h"
#include "engine/engineconfig.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "hid/inputstate.h"
#include "menustate.h"
#include "render/rendersettings.h"
#include "selectedmenustate.h"
#include "ui/ui.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/groupbox.h"
#include "ui/widgets/listbox.h"
#include "ui/widgets/widget.h"

#include <algorithm>
#include <boost/format.hpp>
#include <cmath>
#include <cstdint>
#include <functional>
#include <gl/glad_init.h>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <string>
#include <tuple>
#include <utility>

namespace menu
{
class RenderSettingsMenuState::CheckListBox : public ui::widgets::Widget
{
private:
  gslu::nn_shared<ui::widgets::ListBox> m_listBox;
  ui::widgets::GroupBox m_groupBox;
  std::vector<std::tuple<std::function<bool()>, std::function<void()>, std::shared_ptr<ui::widgets::Checkbox>>>
    m_checkboxes;

public:
  explicit CheckListBox(const std::string& title)
      : m_listBox{std::make_shared<ui::widgets::ListBox>()}
      , m_groupBox{title, m_listBox}
  {
  }

  [[nodiscard]] glm::ivec2 getPosition() const override
  {
    return m_groupBox.getPosition();
  }

  [[nodiscard]] glm::ivec2 getSize() const override
  {
    return m_groupBox.getSize();
  }
  void setPosition(const glm::ivec2& position) override
  {
    m_groupBox.setPosition(position);
  }

  void setSize(const glm::ivec2& size) override
  {
    m_groupBox.setSize(size);
  }

  void update(bool hasFocus) override
  {
    m_groupBox.update(hasFocus);
  }

  void fitToContent() override
  {
    for(const auto& [getter, toggler, checkbox] : m_checkboxes)
      checkbox->fitToContent();

    m_listBox->fitToContent();
    m_groupBox.fitToContent();
  }

  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override
  {
    m_groupBox.draw(ui, presenter);
  }

  auto addSetting(const std::string& name, std::function<bool()>&& getter, std::function<void()>&& toggler)
  {
    auto checkbox = gslu::make_nn_shared<ui::widgets::Checkbox>(name);
    checkbox->setChecked(getter());
    checkbox->fitToContent();
    m_listBox->append(checkbox);
    m_checkboxes.emplace_back(std::move(getter), std::move(toggler), checkbox);
    return checkbox;
  }

  [[nodiscard]] const auto& getSelected() const
  {
    return m_checkboxes.at(m_listBox->getSelected());
  }

  bool nextEntry()
  {
    return m_listBox->nextEntry();
  }

  bool prevEntry()
  {
    return m_listBox->prevEntry();
  }
};

RenderSettingsMenuState::RenderSettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                                 std::unique_ptr<MenuState> previous,
                                                 engine::Engine& engine)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
{
  static const auto toggle = [](engine::Engine& engine, bool& value)
  {
    value = !value;
    engine.applySettings();
  };

  auto listBox = std::make_shared<CheckListBox>(/* translators: TR charmap encoding */ _("Effects"));
  m_listBoxes.emplace_back(listBox);
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("CRT"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.crt;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.crt);
    });
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Depth-of-Field"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.dof;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.dof);
    });
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Lens Distortion"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.lensDistortion;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.lensDistortion);
    });
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Film Grain"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.filmGrain;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.filmGrain);
    });
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Velvia"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.velvia;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.velvia);
    });

  listBox = std::make_shared<CheckListBox>(/* translators: TR charmap encoding */ _("Quality"));
  m_listBoxes.emplace_back(listBox);
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Bilinear Filtering"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.bilinearFiltering;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.bilinearFiltering);
    });
  if(gl::hasAnisotropicFilteringExtension())
  {
    m_anisotropyCheckbox = listBox->addSetting(
      "",
      [&engine]()
      {
        return engine.getEngineConfig()->renderSettings.anisotropyLevel != 0;
      },
      [&engine, maxLevel = gsl::narrow<uint32_t>(std::lround(gl::getMaxAnisotropyLevel()))]()
      {
        auto& level = engine.getEngineConfig()->renderSettings.anisotropyLevel;
        if(level == 0)
          level = 2;
        else
          level *= 2;
        if(level > maxLevel)
          level = 0;
        engine.applySettings();
      });
  }
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Water Denoise"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.waterDenoise;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.waterDenoise);
    });
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("HBAO"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.hbao;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.hbao);
    });
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("FXAA"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.fxaa;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.fxaa);
    });
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("High Quality Shadows"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.highQualityShadows;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.highQualityShadows);
    });
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Half Resolution"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.halfResRender;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.halfResRender);
    });
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Double UI Scale"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.doubleUiScale;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.doubleUiScale);
    });

  listBox = std::make_shared<CheckListBox>(/* translators: TR charmap encoding */ _("Other"));
  m_listBoxes.emplace_back(listBox);
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Fullscreen"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.fullscreen;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.fullscreen);
    });
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Dust"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.dust;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.dust);
    });
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("More Lights"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.moreLights;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.moreLights);
    });
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Ghost"),
    [&engine]()
    {
      return engine.getEngineConfig()->displaySettings.ghost;
    },
    [&engine]()
    {
      auto& b = engine.getEngineConfig()->displaySettings.ghost;
      b = !b;
    });
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Restore Health on Level Start"),
    [&engine]()
    {
      return engine.getEngineConfig()->restoreHealth;
    },
    [&engine]()
    {
      auto& b = engine.getEngineConfig()->restoreHealth;
      b = !b;
    });
}

std::unique_ptr<MenuState>
  RenderSettingsMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& /*display*/)
{
  if(m_anisotropyCheckbox != nullptr)
  {
    m_anisotropyCheckbox->setLabel(/* translators: TR charmap encoding */ _(
      "%1%x Anisotropic Filtering", world.getEngine().getEngineConfig()->renderSettings.anisotropyLevel));
  }

  {
    int maxW = 0;
    int totalH = 0;
    static constexpr int Separation = 10;
    for(const auto& listBox : m_listBoxes)
    {
      listBox->fitToContent();
      maxW = std::max(maxW, listBox->getSize().x);
      totalH += listBox->getSize().y + Separation;
    }

    int y = ui.getSize().y - totalH - 90;
    for(const auto& listBox : m_listBoxes)
    {
      const auto height = listBox->getSize().y;
      listBox->setSize({maxW, height});
      listBox->setPosition({(ui.getSize().x - maxW) / 2, y});
      y += height + Separation;
    }
  }

  for(size_t i = 0; i < m_listBoxes.size(); ++i)
  {
    const auto& listBox = m_listBoxes[i];
    listBox->update(i == m_currentListBox);
    listBox->draw(ui, world.getPresenter());
  }

  const auto& listBox = m_listBoxes[m_currentListBox];
  if(world.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(hid::AxisMovement::Forward))
  {
    if(!listBox->prevEntry() && m_currentListBox > 0)
      --m_currentListBox;
  }
  else if(world.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(hid::AxisMovement::Backward))
  {
    if(!listBox->nextEntry() && m_currentListBox < m_listBoxes.size() - 1)
      ++m_currentListBox;
  }
  if(world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Left))
  {
    if(m_currentListBox > 0)
      --m_currentListBox;
  }
  else if(world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Right))
  {
    if(m_currentListBox < m_listBoxes.size() - 1)
      ++m_currentListBox;
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action))
  {
    const auto& [getter, toggler, checkbox] = listBox->getSelected();
    toggler();
    checkbox->setChecked(getter());
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
  {
    return std::move(m_previous);
  }

  return nullptr;
}
} // namespace menu
