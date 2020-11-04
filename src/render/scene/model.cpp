#include "model.h"

#include "mesh.h"

#include <numeric>

namespace render::scene
{
bool Model::render(RenderContext& context)
{
  context.pushState(getRenderState());
  const auto anyRendered
    = std::accumulate(m_meshes.begin(), m_meshes.end(), false, [&context](bool acc, const auto& mesh) {
        return acc || mesh->render(context);
      });
  context.popState();
  return anyRendered;
}
} // namespace render::scene
