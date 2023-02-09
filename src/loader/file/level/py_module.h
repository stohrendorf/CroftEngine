#include "game.h"

#include <pybind11/pybind11.h>

namespace loader::file::level
{
inline void initLevelModule(pybind11::module& m)
{
  m.doc() = "level loader module";

  auto e = pybind11::enum_<Game>(m, "Game");
#define EXPOSE(name) e.value(#name, Game::name)
  EXPOSE(Unknown);
  EXPOSE(TR1);
  EXPOSE(TR1Demo);
  EXPOSE(TR1UnfinishedBusiness);
  EXPOSE(TR2);
  EXPOSE(TR2Demo);
  EXPOSE(TR2Gold);
  EXPOSE(TR3);
  EXPOSE(TR3Gold);
  EXPOSE(TR4);
  EXPOSE(TR4Demo);
  EXPOSE(TR5);
#undef EXPOSE
}
} // namespace loader::file::level
