#include "ScreenOverlay.h"

#include "Game.h"
#include "Material.h"
#include "MaterialParameter.h"
#include "Model.h"
#include "MeshPart.h"
#include "gl/image.h"

#include <glm/gtc/matrix_transform.hpp>
#include <boost/log/trivial.hpp>


namespace gameplay
{
    namespace
    {
        std::shared_ptr<ShaderProgram> screenOverlayProgram = nullptr;
    }


    ScreenOverlay::ScreenOverlay(Game* game)
        : _game{game}
    {
        BOOST_ASSERT(game != nullptr);

        if( screenOverlayProgram == nullptr )
        {
            screenOverlayProgram = ShaderProgram::createFromFile("shaders/screenoverlay.vert", "shaders/screenoverlay.frag", {});
            if( screenOverlayProgram == nullptr )
            {
                BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create effect for screen overlay."));
            }
        }

        resize();
    }


    void ScreenOverlay::resize()
    {
        // Update the projection matrix for our batch to match the current viewport
        const Rectangle& vp = _game->getViewport();

        if(vp.isEmpty())
        {
            BOOST_THROW_EXCEPTION(std::runtime_error("Cannot create screen overlay because the viewport is empty"));
        }

        _image = std::make_shared<gl::Image<gl::RGBA8>>(static_cast<GLint>(vp.width), static_cast<GLint>(vp.height));
        _image->fill({0,0,0,0});

        _texture = std::make_shared<gl::Texture>(GL_TEXTURE_2D);
        _texture->image2D(_image->getWidth(), _image->getHeight(), _image->getData(), false);
        _texture->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        _texture->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        _mesh = Mesh::createQuadFullscreen(vp.width, vp.height, screenOverlayProgram->getHandle(), true);
        auto part = _mesh->getPart(0);
        part->setMaterial(std::make_shared<Material>(screenOverlayProgram));
        part->getMaterial()->getParameter("u_texture")->set(_texture);
        part->getMaterial()->getParameter("u_projectionMatrix")->set(glm::ortho(vp.x, vp.width, vp.height, vp.y, 0.0f, 1.0f));
        part->getMaterial()->getStateBlock()->setBlend(true);
        part->getMaterial()->getStateBlock()->setBlendSrc(RenderState::BLEND_SRC_ALPHA);
        part->getMaterial()->getStateBlock()->setBlendDst(RenderState::BLEND_ONE_MINUS_SRC_ALPHA);

        _model = std::make_shared<Model>();
        _model->addMesh(_mesh);
    }


    ScreenOverlay::~ScreenOverlay() = default;


    void ScreenOverlay::draw(RenderContext& context)
    {
        _texture->subImage2D(_image->getData());
        _model->draw(context);
    }
}
