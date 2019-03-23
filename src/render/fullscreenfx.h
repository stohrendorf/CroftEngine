#pragma once

#include "gl/framebuffer.h"
#include "scene/Scene.h"
#include "render/scene/mesh.h"
#include "scene/window.h"

namespace render
{
class FullScreenFX
{
    std::shared_ptr<gl::Texture> m_depthBuffer{nullptr};
    std::shared_ptr<gl::Texture> m_colorBuffer{nullptr};
    const gsl::not_null<std::shared_ptr<scene::ShaderProgram>> m_shader;
    const gsl::not_null<std::shared_ptr<scene::Material>> m_material;

public:
    explicit FullScreenFX(const scene::Window& window,
                          gsl::not_null<std::shared_ptr<scene::ShaderProgram>> shader)
            : m_shader{std::move( shader )}
            , m_material{std::make_shared<scene::Material>( m_shader )}
            , m_fb{std::make_shared<gl::FrameBuffer>()}
    {
        init( window );
    }

    void init(const scene::Window& window)
    {
        const auto vp = window.getViewport();

        m_colorBuffer = std::make_shared<gl::Texture>( GL_TEXTURE_2D );
        m_colorBuffer->image2D<gl::RGBA8>(
                gsl::narrow<GLint>( vp.width ), gsl::narrow<GLint>( vp.height ), false );
        m_fb->attachTexture2D( GL_COLOR_ATTACHMENT0, *m_colorBuffer );

        m_depthBuffer = std::make_shared<gl::Texture>( GL_TEXTURE_2D );
        m_depthBuffer->depthImage2D( gsl::narrow<GLint>( vp.width ), gsl::narrow<GLint>( vp.height ) );
        m_fb->attachTexture2D( GL_DEPTH_ATTACHMENT, *m_depthBuffer );

        BOOST_ASSERT( m_fb->isComplete() );

        m_mesh = scene::createQuadFullscreen( gsl::narrow<float>( vp.width ), gsl::narrow<float>( vp.height ), m_shader->getHandle() );
        m_material->getParameter( "u_depth" )->set( m_depthBuffer );
        m_material->getParameter( "u_projectionMatrix" )
                  ->set( glm::ortho( 0.0f, gsl::narrow<float>( vp.width ), gsl::narrow<float>( vp.height ), 0.0f, 0.0f,
                                     1.0f ) );
        m_material->getParameter( "u_projection" )->bindProjectionMatrix();
        m_material->getParameter( "u_texture" )->set( m_colorBuffer );

        m_mesh->setMaterial( m_material );

        m_model = std::make_shared<scene::Model>();
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

    void render(scene::RenderContext& context) const
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        gl::checkGlError();

        m_model->draw( context );
    }

    const gsl::not_null<std::shared_ptr<scene::Material>>& getMaterial() const
    {
        return m_material;
    }

private:
    std::shared_ptr<gl::FrameBuffer> m_fb;
    std::shared_ptr<scene::Mesh> m_mesh;
    std::shared_ptr<scene::Model> m_model;
};
}