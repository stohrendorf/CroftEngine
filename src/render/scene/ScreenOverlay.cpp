#include "ScreenOverlay.h"

#include "Material.h"
#include "MaterialParameter.h"
#include "mesh.h"
#include "model.h"
#include "render/gl/image.h"
#include "renderer.h"

#include <boost/log/trivial.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace render
{
namespace scene
{
ScreenOverlay::ScreenOverlay(const Dimension2<size_t>& viewport)
{
    init(viewport);
}

ScreenOverlay::~ScreenOverlay() = default;

void ScreenOverlay::render(RenderContext& context)
{
    context.pushState(getRenderState());
    m_texture->subImage2D(m_image->getData());
    m_model->render(context);
    context.popState();
}

void ScreenOverlay::init(const Dimension2<size_t>& viewport)
{
    *m_image = gl::Image<gl::SRGBA8>(gsl::narrow<int32_t>(viewport.width), gsl::narrow<int32_t>(viewport.height));
    if(viewport.width <= 0 || viewport.height <= 0)
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("Cannot create screen overlay because the viewport is empty"));
    }

    const auto screenOverlayProgram
        = ShaderProgram::createFromFile("shaders/screenoverlay.vert", "shaders/screenoverlay.frag", {});

    m_texture->image2D(m_image->getWidth(), m_image->getHeight(), m_image->getData());
    m_texture->set(::gl::TextureMinFilter::Nearest)
        .set(::gl::TextureMagFilter::Nearest)
        .set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToEdge)
        .set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToEdge);

    m_mesh = render::scene::createQuadFullscreen(gsl::narrow<float>(viewport.width),
                                                 gsl::narrow<float>(viewport.height),
                                                 screenOverlayProgram->getHandle(),
                                                 true);
    m_mesh->setMaterial(std::make_shared<Material>(screenOverlayProgram));
    m_mesh->getMaterial()->getParameter("u_texture")->set(m_texture);
    m_mesh->getMaterial()
        ->getParameter("u_projection")
        ->set(glm::ortho(
            0.0f, gsl::narrow<float>(viewport.width), gsl::narrow<float>(viewport.height), 0.0f, 0.0f, 1.0f));

    m_model->getMeshes().clear();
    m_model->addMesh(m_mesh);

    m_model->getRenderState().setCullFace(false);
    m_model->getRenderState().setDepthWrite(false);
    m_model->getRenderState().setDepthTest(false);
}
} // namespace scene
} // namespace render
