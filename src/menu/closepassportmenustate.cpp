#include "closepassportmenustate.h"

#include "core/units.h"
#include "engine/items_tr1.h"
#include "finishitemanimationmenustate.h"
#include "menuobject.h"
#include "menustate.h"
#include "passportmenustate.h"
#include "qs/qs.h"
#include "resetitemtransformmenustate.h"
#include "setitemtypemenustate.h"

#include <gsl-lite/gsl-lite.hpp>
#include <memory>
#include <utility>

namespace menu
{
ClosePassportMenuState::ClosePassportMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                               MenuObject& passport,
                                               std::unique_ptr<MenuState>&& next)
    : MenuState{ringTransform}
    , m_next{std::move(next)}
{
  gsl_Expects(passport.type == engine::TR1ItemId::PassportOpening);

  const auto localFrame = passport.goalFrame - passport.openFrame;
  if(const auto page = localFrame / PassportMenuState::FramesPerPage; page == PassportMenuState::ExitGamePage)
  {
    passport.goalFrame = passport.lastMeshAnimFrame - 1_frame;
    passport.animDirection = 1_frame;
  }
  else
  {
    passport.goalFrame = 0_frame;
    passport.animDirection = -1_frame;
  }
}

std::unique_ptr<MenuState>
  ClosePassportMenuState::onFrame(ui::Ui& /*ui*/, engine::world::World& /*world*/, MenuDisplay& /*display*/)
{
  return create<FinishItemAnimationMenuState>(create<SetItemTypeMenuState>(
    engine::TR1ItemId::PassportClosed, create<ResetItemTransformMenuState>(std::move(m_next))));
}

void ClosePassportMenuState::handleObject(ui::Ui& /*ui*/,
                                          engine::world::World& /*world*/,
                                          MenuDisplay& /*display*/,
                                          MenuObject& /*object*/)
{
}
} // namespace menu