#include "closepassportmenustate.h"

#include "engine/items_tr1.h"
#include "finishitemanimationmenustate.h"
#include "idleringmenustate.h"
#include "menuobject.h"
#include "passportmenustate.h"
#include "resetitemtransformmenustate.h"
#include "setitemtypemenustate.h"

namespace menu
{
ClosePassportMenuState::ClosePassportMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                               MenuObject& passport)
    : MenuState{ringTransform}
{
  Expects(passport.type == engine::TR1ItemId::PassportOpening);

  const auto localFrame = passport.goalFrame - passport.openFrame;
  auto page = localFrame / PassportMenuState::FramesPerPage;
  if(page == PassportMenuState::ExitGamePage)
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
  ClosePassportMenuState::onFrame(ui::Ui& /*ui*/, engine::World& /*world*/, MenuDisplay& /*display*/)
{
  return create<FinishItemAnimationMenuState>(create<SetItemTypeMenuState>(
    engine::TR1ItemId::PassportClosed, create<ResetItemTransformMenuState>(create<IdleRingMenuState>(false))));
}

void ClosePassportMenuState::handleObject(ui::Ui& /*ui*/,
                                          engine::World& /*world*/,
                                          MenuDisplay& /*display*/,
                                          MenuObject& /*object*/)
{
}
} // namespace menu
