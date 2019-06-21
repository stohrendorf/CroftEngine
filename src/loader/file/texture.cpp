#include "texture.h"

#include "engine/items/itemnode.h"
#include "loader/trx/trx.h"
#include "util/cimgwrapper.h"

#include <boost/range/adaptor/indexed.hpp>

namespace loader
{
namespace file
{
gsl::not_null<std::shared_ptr<render::scene::Material>>
    createMaterial(const gsl::not_null<std::shared_ptr<render::gl::Texture>>& texture,
                   const BlendingMode bmode,
                   const gsl::not_null<std::shared_ptr<render::scene::ShaderProgram>>& shader)
{
    auto result = std::make_shared<render::scene::Material>(shader);
    // Set some defaults
    texture->set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToEdge);
    texture->set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToEdge);
    result->getParameter("u_diffuseTexture")->set(texture);
    result->getParameter("u_modelMatrix")->bindModelMatrix();
    result->getParameter("u_modelViewMatrix")->bindModelViewMatrix();
    result->getParameter("u_camProjection")->bindProjectionMatrix();

    switch(bmode)
    {
    case BlendingMode::Solid:
    case BlendingMode::AlphaTransparency:
    case BlendingMode::VertexColorTransparency: // Classic PC alpha
    case BlendingMode::InvertSrc:               // Inversion by src (PS darkness) - SAME AS IN TR3-TR5
    case BlendingMode::InvertDst:               // Inversion by dest
    case BlendingMode::Screen:                  // Screen (smoke, etc.)
    case BlendingMode::AnimatedTexture: break;

    default:                 // opaque animated textures case
        BOOST_ASSERT(false); // FIXME
    }

    return result;
}

void DWordTexture::toImage(const trx::Glidos* glidos, const std::function<void(const std::string&)>& statusCallback)
{
    if(glidos == nullptr)
    {
        image = std::make_shared<render::gl::Image<render::gl::SRGBA8>>(256, 256, &pixels[0][0]);
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "Upgrading texture " << md5 << "...";

    constexpr int Resolution = 2048;
    constexpr int Scale = Resolution / 256;

    auto mapping = glidos->getMappingsForTexture(md5);
    const auto cacheName = mapping.baseDir / "_edisonengine" / (md5 + ".png");

    if(is_regular_file(cacheName)
       && std::chrono::system_clock::from_time_t(last_write_time(cacheName)) > mapping.newestSource)
    {
        statusCallback("Loading cached texture...");
        BOOST_LOG_TRIVIAL(info) << "Loading cached texture " << cacheName << "...";
        util::CImgWrapper cacheImage{cacheName.string()};

        cacheImage.interleave();
        image = std::make_shared<render::gl::Image<render::gl::SRGBA8>>(
            cacheImage.width(), cacheImage.height(), reinterpret_cast<const render::gl::SRGBA8*>(cacheImage.data()));
        return;
    }

    statusCallback("Upgrading texture (upscaling)");
    util::CImgWrapper original{&pixels[0][0].r, 256, 256, false};
    original.deinterleave();
    original.resize(Resolution, Resolution);

    for(const auto& indexedTile : mapping.tiles | boost::adaptors::indexed(0))
    {
        const auto& tile = indexedTile.value();
        statusCallback("Upgrading texture (" + std::to_string(indexedTile.index() * 100 / mapping.tiles.size()) + "%)");

        BOOST_LOG_TRIVIAL(info) << "  - Loading " << tile.second << " into " << tile.first;
        if(!is_regular_file(tile.second))
        {
            BOOST_LOG_TRIVIAL(warning) << "    File not found";
            continue;
        }

        util::CImgWrapper srcImage{tile.second.string()};
        srcImage.resize(tile.first.getWidth() * Scale, tile.first.getHeight() * Scale);
        const auto x0 = tile.first.getX0() * Scale;
        const auto y0 = tile.first.getY0() * Scale;
        for(int x = 0; x < srcImage.width(); ++x)
        {
            for(int y = 0; y < srcImage.height(); ++y)
            {
                BOOST_ASSERT(x + static_cast<int>(x0) < original.width());
                BOOST_ASSERT(y + static_cast<int>(y0) < original.height());

                for(int c = 0; c < 4; ++c)
                {
                    original(x + x0, y + y0, c) = srcImage(x, y, c);
                }
            }
        }
    }

    statusCallback("Saving texture to cache...");
    BOOST_LOG_TRIVIAL(info) << "Writing texture cache " << cacheName << "...";
    create_directories(cacheName.parent_path());
    original.savePng(cacheName.string());

    original.interleave();
    image = std::make_shared<render::gl::Image<render::gl::SRGBA8>>(
        Resolution, Resolution, reinterpret_cast<const render::gl::SRGBA8*>(original.data()));
}

void DWordTexture::toTexture(const trx::Glidos* glidos, const std::function<void(const std::string&)>& statusCallback)
{
    texture = std::make_shared<render::gl::Texture2D<render::gl::SRGBA8>>(md5);
    if(glidos == nullptr)
    {
        texture->set(::gl::TextureMinFilter::Linear).set(::gl::TextureMagFilter::Nearest);
    }
    else
    {
        texture->set(::gl::TextureMinFilter::Linear).set(::gl::TextureMagFilter::Linear);
    }
    toImage(glidos, statusCallback);
    texture->image(image->getWidth(), image->getHeight(), image->getData()).generateMipmap();
}
} // namespace file
} // namespace loader
