#include "model.h"

#include "mesh.h"
#include "names.h"

namespace render
{
namespace scene
{
void Model::render(RenderContext& context)
{
    context.pushState(getRenderState());
    for(const auto& mesh : m_meshes)
    {
        mesh->render(context);
    }
    context.popState();
}
} // namespace scene
} // namespace render
