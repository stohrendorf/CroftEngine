#include "selectlevelmenustate.h"

#include "closepassportmenustate.h"
#include "core/i18n.h"
#include "engine/engine.h"
#include "engine/script/reflection.h"
#include "engine/world/world.h"
#include "menudisplay.h"
#include "menuring.h"
#include "requestlevelmenustate.h"
#include "ui/widgets/label.h"

#include <boost/range/adaptor/indexed.hpp>

namespace menu
{
SelectLevelMenuState::SelectLevelMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                           std::unique_ptr<MenuState> previous,
                                           const engine::world::World& world)
    : ListDisplayMenuState{ringTransform, /* translators: TR charmap encoding */ _("Select Level"), 20}
    , m_previous{std::move(previous)}
{
  auto appendLabel = [this](const std::string& title)
  {
    append(gsl::make_shared<ui::widgets::Label>(title, ui::widgets::Label::Alignment::Center));
  };

  for(const auto& item :
      world.getEngine().getScriptEngine().getGameflow().getLevelSequence() | boost::adaptors::indexed())
  {
    const auto level = std::dynamic_pointer_cast<engine::script::Level>(item.value());
    if(level == nullptr)
      continue;

    const auto titles = level->getTitles();
    auto titleIt = titles.find(world.getEngine().getLocaleWithoutEncoding());
    if(titleIt == titles.end())
    {
      titleIt = titles.find("en_GB");
    }
    gsl_Assert(titleIt != titles.end());
    appendLabel(titleIt->second);
    m_indices.emplace_back(item.index());
  }
}

std::unique_ptr<MenuState> SelectLevelMenuState::onAborted()
{
  return std::move(m_previous);
}

std::unique_ptr<MenuState>
  SelectLevelMenuState::onSelected(size_t idx, engine::world::World& /*world*/, MenuDisplay& display)
{
  return create<ClosePassportMenuState>(display.getCurrentRing().getSelectedObject(),
                                        create<RequestLevelMenuState>(m_indices.at(idx)));
}
} // namespace menu
