#include "menuring.h"

#include "engine/engine.h"
#include "engine/objects/laraobject.h"
#include "menudisplay.h"

#include <utility>

namespace menu
{
MenuRing::MenuRing(Type type, std::string title, std::vector<MenuObject> list)
    : title{std::move(title)}
    , type{type}
    , list{std::move(list)}
{
}
} // namespace menu
