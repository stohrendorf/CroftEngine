#include "rendersettingsmenustate.h"

#include "engine/engine.h"
#include "engine/i18n.h"
#include "engine/presenter.h"
#include "engine/world.h"
#include "menudisplay.h"
#include "ui/label.h"

namespace menu
{
namespace
{
constexpr auto green(uint8_t g)
{
  return gl::SRGB8{0, g, 0};
}

void setEnabledBackground(ui::Label& lbl, bool enabled)
{
  if(enabled)
  {
    lbl.addBackground(glm::ivec2{RenderSettingsMenuState::PixelWidth - 12, 16}, {0, 0});
    lbl.backgroundGouraud = ui::Label::makeBackgroundCircle(green(255), 255, 32);
  }
  else
  {
    lbl.removeBackground();
  }
}
} // namespace

RenderSettingsMenuState::RenderSettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                                 std::unique_ptr<MenuState> previous)
    : MenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_background{std::make_unique<ui::Label>(glm::ivec2{0, YOffset - 12}, " ")}
{
  m_background->alignX = ui::Label::Alignment::Center;
  m_background->alignY = ui::Label::Alignment::Bottom;
  m_background->addBackground({PixelWidth, TotalHeight + 12}, {0, 0});
  m_background->backgroundGouraud = ui::Label::makeBackgroundCircle(green(32 * 3), 255, 96);
  m_background->outline = true;

  auto addSetting = [this](const std::string& name, bool enabled) {
    auto lbl = std::make_shared<ui::Label>(glm::ivec2{0, YOffset + m_labels.size() * LineHeight}, name);
    lbl->alignX = ui::Label::Alignment::Center;
    lbl->alignY = ui::Label::Alignment::Bottom;
    setEnabledBackground(*lbl, true); // needed to initialize background size for outlining
    setEnabledBackground(*lbl, enabled);
    m_labels.emplace_back(lbl, enabled);
  };

  addSetting("CRT", false);
  addSetting("Depth-of-Field", false);
  addSetting("Lens Distortion", false);
  addSetting("SSAO", false);
  addSetting("FXAA", false);
  addSetting("Film Grain", false);
}

void RenderSettingsMenuState::handleObject(engine::World& /*world*/, MenuDisplay& /*display*/, MenuObject& /*object*/)
{
}

std::unique_ptr<MenuState>
  RenderSettingsMenuState::onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& /*display*/)
{
  m_background->draw(world.getPresenter().getTrFont(), img, world.getPalette());

  for(size_t i = 0; i < m_labels.size(); ++i)
  {
    const auto& [lbl, enabled] = m_labels.at(i);
    if(m_selected == i)
    {
      lbl->outline = true;
    }
    else
    {
      lbl->outline = false;
    }
    lbl->draw(world.getPresenter().getTrFont(), img, world.getPalette());
  }

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
  else if(world.getPresenter().getInputHandler().getInputState().action.justChangedTo(true))
  {
    auto& entry = m_labels.at(m_selected);
    entry.second = !entry.second;
    setEnabledBackground(*entry.first, entry.second);
  }
  else if(world.getPresenter().getInputHandler().getInputState().menu.justChangedTo(true))
  {
    return std::move(m_previous);
  }

  return nullptr;
}
} // namespace menu
