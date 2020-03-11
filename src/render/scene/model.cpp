#include "model.h"

#include "mesh.h"

namespace render::scene
{
bool Model::render(RenderContext& context)
{
  bool anyRendered = false;
  context.pushState(getRenderState());
  for(const auto& mesh : m_meshes)
  {
    anyRendered |= mesh->render(context);
  }
  context.popState();
  return anyRendered;
}
} // namespace render::scene
