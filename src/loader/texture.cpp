#include "texture.h"

#include "engine/items/itemnode.h"
#include "loader/trx/trx.h"

#include <glm/gtc/type_ptr.hpp>

#ifdef _X
#undef _X
#endif

#include "CImg.h"

namespace loader
{
gsl::not_null<std::shared_ptr<gameplay::Material>> createMaterial(
        const gsl::not_null<std::shared_ptr<gameplay::gl::Texture>>& texture,
        BlendingMode bmode,
        const gsl::not_null<std::shared_ptr<gameplay::ShaderProgram>>& shader)
{
    auto result = make_not_null_shared<gameplay::Material>( shader );
    // Set some defaults
    texture->set( GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    texture->set( GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    result->getParameter( "u_diffuseTexture" )->set( texture );
    result->getParameter( "u_modelMatrix" )->bindModelMatrix();
    result->getParameter( "u_modelViewMatrix" )->bindModelViewMatrix();
    result->getParameter( "u_projectionMatrix" )->bindProjectionMatrix();

    switch( bmode )
    {
        case BlendingMode::Solid:
        case BlendingMode::AlphaTransparency:
        case BlendingMode::VertexColorTransparency: // Classic PC alpha
        case BlendingMode::InvertSrc: // Inversion by src (PS darkness) - SAME AS IN TR3-TR5
        case BlendingMode::InvertDst: // Inversion by dest
        case BlendingMode::Screen: // Screen (smoke, etc.)
        case BlendingMode::AnimatedTexture:
            break;

        default: // opaque animated textures case
            BOOST_ASSERT( false ); // FIXME
    }

    return result;
}

void DWordTexture::toImage(
        trx::Glidos* glidos,
        const boost::filesystem::path& lvlName)
{
    if( glidos == nullptr )
    {
        image = std::make_shared<gameplay::gl::Image<gameplay::gl::RGBA8>>( 256, 256, &pixels[0][0] );
        return;
    }

    BOOST_LOG_TRIVIAL( info ) << "Upgrading texture " << md5 << "...";

    constexpr int Resolution = 2048;
    constexpr int Scale = Resolution / 256;

    auto mapping = glidos->getMappingsForTexture( md5 );
    const auto cacheName =  mapping.baseDir / "_edisonengine" / lvlName / (md5 + ".png");

    if( is_regular_file( cacheName ) &&
        std::chrono::system_clock::from_time_t( last_write_time( cacheName ) ) > mapping.newestSource )
    {
        BOOST_LOG_TRIVIAL( info ) << "Loading cached texture " << cacheName << "...";
        cimg_library::CImg<uint8_t> cacheImage( cacheName.string().c_str() );

        if( cacheImage.spectrum() == 3 )
        {
            cacheImage.channels( 0, 3 );
            BOOST_ASSERT( cacheImage.spectrum() == 4 );
            cacheImage.get_shared_channel( 3 ).fill( 1 );
        }

        if( cacheImage.spectrum() != 4 )
        {
            BOOST_THROW_EXCEPTION( std::runtime_error( "Can only use RGB and RGBA images" ) );
        }

        const auto w = cacheImage.width();
        const auto h = cacheImage.height();

        // interleave
        cacheImage.permute_axes( "cxyz" );

        image = std::make_shared<gameplay::gl::Image<gameplay::gl::RGBA8>>(
                w, h,
                reinterpret_cast<const gameplay::gl::RGBA8*>(cacheImage.data()) );
        return;
    }

    cimg_library::CImg<uint8_t> original( &pixels[0][0].r, 4, 256, 256, 1, false );
    // un-interleave
    original.permute_axes( "yzcx" );
    BOOST_ASSERT( original.width() == 256 && original.height() == 256 && original.spectrum() == 4 );
    original.resize( Resolution, Resolution, 1, 4, 6 );
    original.min( uint8_t( 255 ) ).max( uint8_t( 0 ) ); // interpolation may produce values outside the range 0..255
    BOOST_ASSERT( original.width() == Resolution && original.height() == Resolution && original.spectrum() == 4 );

    for( const auto& tile : mapping.tiles )
    {
        BOOST_LOG_TRIVIAL( info ) << "  - Loading " << tile.second << " into " << tile.first;
        if( !is_regular_file( tile.second ) )
        {
            BOOST_LOG_TRIVIAL( warning ) << "    File not found";
            continue;
        }

        cimg_library::CImg<uint8_t> srcImage( tile.second.string().c_str() );

        if( srcImage.spectrum() == 3 )
        {
            srcImage.channels( 0, 3 );
            BOOST_ASSERT( srcImage.spectrum() == 4 );
            srcImage.get_shared_channel( 3 ).fill( 255 );
        }

        if( srcImage.spectrum() != 4 )
        {
            BOOST_THROW_EXCEPTION( std::runtime_error( "Can only use RGB and RGBA images" ) );
        }

        srcImage.resize( tile.first.getWidth() * Scale, tile.first.getHeight() * Scale, 1, 4, 6 );
        const auto x0 = tile.first.getX0() * Scale;
        const auto y0 = tile.first.getY0() * Scale;
        for( int x = 0; x < srcImage.width(); ++x )
        {
            for( int y = 0; y < srcImage.height(); ++y )
            {
                BOOST_ASSERT( x + static_cast<int>(x0) < original.width() );
                BOOST_ASSERT( y + static_cast<int>(y0) < original.height() );

                for( int c = 0; c < 4; ++c )
                {
                    const auto pixel = srcImage( x, y, 0, c );
                    original( x + x0, y + y0, 0, c ) = pixel;
                }
            }
        }
    }

    BOOST_LOG_TRIVIAL( info ) << "Writing texture cache " << cacheName << "...";
    boost::filesystem::create_directories( cacheName.parent_path() );
    original.save_png( cacheName.string().c_str(), 1 );

    // interleave
    original.permute_axes( "cxyz" );

    image = std::make_shared<gameplay::gl::Image<gameplay::gl::RGBA8>>(
            Resolution, Resolution,
            reinterpret_cast<const gameplay::gl::RGBA8*>(original.data()) );
}

void DWordTexture::toTexture(
        trx::Glidos* glidos,
        const boost::filesystem::path& lvlName)
{
    texture = make_not_null_shared<gameplay::gl::Texture>( GL_TEXTURE_2D );
    texture->setLabel( md5 );
    toImage( glidos, lvlName );
    texture->image2D( image->getWidth(), image->getHeight(), image->getData(), true );
}
}
