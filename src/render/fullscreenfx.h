#pragma once

#include "gameplay.h"
#include "gl/framebuffer.h"

namespace render
{
class FullScreenFX
{
    std::shared_ptr<gameplay::gl::Texture> m_depthBuffer{nullptr};
    std::shared_ptr<gameplay::gl::Texture> m_colorBuffer{nullptr};
    const gsl::not_null<std::shared_ptr<gameplay::ShaderProgram>> m_shader;
    const gsl::not_null<std::shared_ptr<gameplay::Material>> m_material;

public:
    explicit FullScreenFX(const gameplay::Game& game,
                          gsl::not_null<std::shared_ptr<gameplay::ShaderProgram>> shader)
            : m_shader{std::move( shader )}
            , m_material{std::make_shared<gameplay::Material>( m_shader )}
            , m_fb{std::make_shared<gameplay::gl::FrameBuffer>()}
    {
        init( game );
    }

    void init(const gameplay::Game& game)
    {
        const auto vp = game.getViewport();

        m_colorBuffer = std::make_shared<gameplay::gl::Texture>( GL_TEXTURE_2D );
        m_colorBuffer->image2D<gameplay::gl::RGBA8>(
                gsl::narrow<GLint>( vp.width ), gsl::narrow<GLint>( vp.height ), false );
        m_fb->attachTexture2D( GL_COLOR_ATTACHMENT0, *m_colorBuffer );

        m_depthBuffer = std::make_shared<gameplay::gl::Texture>( GL_TEXTURE_2D );
        m_depthBuffer->depthImage2D( gsl::narrow<GLint>( vp.width ), gsl::narrow<GLint>( vp.height ) );
        m_fb->attachTexture2D( GL_DEPTH_ATTACHMENT, *m_depthBuffer );

        BOOST_ASSERT( m_fb->isComplete() );

        m_mesh = gameplay::Mesh::createQuadFullscreen( gsl::narrow<float>( vp.width ), gsl::narrow<float>( vp.height ),
                                                       m_shader->getHandle() );
        m_material->getParameter( "u_depth" )->set( m_depthBuffer );
        m_material->getParameter( "u_projectionMatrix" )
                  ->set( glm::ortho( 0.0f, gsl::narrow<float>( vp.width ), gsl::narrow<float>( vp.height ), 0.0f, 0.0f,
                                     1.0f ) );
        m_material->getParameter( "u_projection" )
                  ->bind( game.getScene()->getActiveCamera().get(), &gameplay::Camera::getProjectionMatrix );
        m_material->getParameter( "u_texture" )->set( m_colorBuffer );

        m_mesh->getParts()[0]->setMaterial( m_material );

        m_model = std::make_shared<gameplay::Model>();
        m_model->addMesh( m_mesh );

        m_model->getRenderState().setDepthWrite( false );
        m_model->getRenderState().setDepthTest( false );
        m_model->getRenderState().setCullFace( false );

        m_colorBuffer->set( GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        m_colorBuffer->set( GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    }

    void bind() const
    {
        m_fb->bind();
    }

    void render(gameplay::RenderContext& context) const
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        gameplay::gl::checkGlError();

        m_model->draw( context );
    }

    const gsl::not_null<std::shared_ptr<gameplay::Material>>& getMaterial() const
    {
        return m_material;
    }

private:
    std::shared_ptr<gameplay::gl::FrameBuffer> m_fb;
    std::shared_ptr<gameplay::Mesh> m_mesh;
    std::shared_ptr<gameplay::Model> m_model;
};
}