#include "ScreenOverlay.h"

#include "Game.h"
#include "Material.h"
#include "MaterialParameter.h"
#include "Model.h"
#include "MeshPart.h"
#include "gl/image.h"
#include "gsl_util.h"

#include <glm/gtc/matrix_transform.hpp>
#include <boost/log/trivial.hpp>

namespace gameplay
{
ScreenOverlay::ScreenOverlay(const Rectangle& viewport)
        : m_image{std::make_shared<gl::Image<gl::RGBA8>>( static_cast<GLint>(viewport.width),
                                                          static_cast<GLint>(viewport.height) )}
{
    // Update the projection matrix for our batch to match the current viewport
    if( viewport.isEmpty() )
    {
        BOOST_THROW_EXCEPTION( std::runtime_error( "Cannot create screen overlay because the viewport is empty" ) );
    }

    auto screenOverlayProgram = to_not_null( ShaderProgram::createFromFile( "shaders/screenoverlay.vert",
                                                                            "shaders/screenoverlay.frag", {} ) );

    m_image->fill( {0, 0, 0, 0} );

    m_texture->image2D( m_image->getWidth(), m_image->getHeight(), m_image->getData(), false );
    m_texture->set( GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    m_texture->set( GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    m_mesh = Mesh::createQuadFullscreen( viewport.width, viewport.height, screenOverlayProgram->getHandle(), true );
    auto part = m_mesh->getPart( 0 );
    part->setMaterial( std::make_shared<Material>( screenOverlayProgram ) );
    part->getMaterial()->getParameter( "u_texture" )->set( m_texture );
    part->getMaterial()->getParameter( "u_projectionMatrix" )
        ->set( glm::ortho( viewport.x, viewport.width, viewport.height, viewport.y, 0.0f, 1.0f ) );

    m_model->addMesh( to_not_null( m_mesh ) );
}

ScreenOverlay::~ScreenOverlay() = default;

void ScreenOverlay::draw(RenderContext& context)
{
    m_texture->subImage2D( m_image->getData() );
    m_model->draw( context );
}
}
