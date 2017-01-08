#include "texture.h"

#include "engine/items/itemnode.h"
#include "loader/trx/trx.h"

#include <glm/gtc/type_ptr.hpp>

#include "CImg.h"



namespace loader
{
    std::shared_ptr<gameplay::Material> createMaterial(const std::shared_ptr<gameplay::Texture>& texture, BlendingMode bmode,
                                                       const std::shared_ptr<gameplay::ShaderProgram>& shader)
    {
        auto result = std::make_shared<gameplay::Material>(shader);
        // Set some defaults
        auto sampler = std::make_shared<gameplay::Texture::Sampler>(texture);
        sampler->setWrapMode(gameplay::Texture::CLAMP, gameplay::Texture::CLAMP);
        result->getParameter("u_diffuseTexture")->set(sampler);
        result->getParameter("u_worldViewProjectionMatrix")->bindWorldViewProjectionMatrix();
        result->getParameter("u_modelMatrix")->bindModelMatrix();
        result->getParameter("u_baseLight")->bind(&engine::items::ItemNode::lightBaseBinder);
        result->getParameter("u_baseLightDiff")->bind(&engine::items::ItemNode::lightBaseDiffBinder);
        result->getParameter("u_lightPosition")->bind(&engine::items::ItemNode::lightPositionBinder);
        result->initStateBlockDefaults();


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
                BOOST_ASSERT(false); // FIXME [irrlicht]
        }

        return result;
    }


    std::shared_ptr<gameplay::Image> DWordTexture::toImage(trx::Glidos* glidos, const boost::filesystem::path& lvlName) const
    {
        if( glidos == nullptr )
        {
            return std::make_shared<gameplay::Image>(256, 256, &pixels[0][0]);
        }

        BOOST_LOG_TRIVIAL(info) << "Upgrading texture " << md5 << "...";

        constexpr int Resolution = 2048;
        constexpr int Scale = Resolution / 256;

        auto mapping = glidos->getMappingsForTexture(md5);
        const auto cacheName = mapping.baseDir / "_edisonengine" / lvlName / (md5 + ".png");

        if( boost::filesystem::is_regular_file(cacheName) &&
            std::chrono::system_clock::from_time_t(boost::filesystem::last_write_time(cacheName)) > mapping.newestSource )
        {
            BOOST_LOG_TRIVIAL(info) << "Loading cached texture " << cacheName << "...";
            cimg_library::CImg<float> cacheImage(cacheName.string().c_str());
            cacheImage /= 255;

            if( cacheImage.spectrum() == 3 )
            {
                cacheImage.channels(0, 3);
                BOOST_ASSERT(cacheImage.spectrum() == 4);
                cacheImage.get_shared_channel(3).fill(1);
            }

            if( cacheImage.spectrum() != 4 )
            {
                BOOST_THROW_EXCEPTION(std::runtime_error("Can only use RGB and RGBA images"));
            }

            const auto w = cacheImage.width();
            const auto h = cacheImage.height();

            // interleave
            cacheImage.permute_axes("cxyz");

            return std::make_shared<gameplay::Image>(w, h, reinterpret_cast<const glm::vec4*>(cacheImage.data()));
        }

        cimg_library::CImg<float> original(glm::value_ptr(pixels[0][0]), 4, 256, 256, 1, false);
        // un-interleave
        original.permute_axes("yzcx");
        BOOST_ASSERT(original.width() == 256 && original.height() == 256 && original.spectrum() == 4);
        original.resize(Resolution, Resolution, 1, 4, 6);
        original.min(1.0f).max(0.0f); // interpolation may produce values outside the range 0..1
        BOOST_ASSERT(original.width() == Resolution && original.height() == Resolution && original.spectrum() == 4);

        for( const auto& tile : mapping.tiles )
        {
            BOOST_LOG_TRIVIAL(info) << "  - Loading " << tile.second << " into " << tile.first;
            if( !boost::filesystem::is_regular_file(tile.second) )
            {
                BOOST_LOG_TRIVIAL(warning) << "    File not found";
                continue;
            }

            cimg_library::CImg <uint8_t> srcImage(tile.second.string().c_str());

            if( srcImage.spectrum() == 3 )
            {
                srcImage.channels(0, 3);
                BOOST_ASSERT(srcImage.spectrum() == 4);
                srcImage.get_shared_channel(3).fill(255);
            }

            if( srcImage.spectrum() != 4 )
            {
                BOOST_THROW_EXCEPTION(std::runtime_error("Can only use RGB and RGBA images"));
            }

            srcImage.resize(tile.first.getWidth() * Scale, tile.first.getHeight() * Scale, 1, 4, 6);
            const auto x0 = tile.first.getX0() * Scale;
            const auto y0 = tile.first.getY0() * Scale;
            for( int x = 0; x < srcImage.width(); ++x )
            {
                for( int y = 0; y < srcImage.height(); ++y )
                {
                    BOOST_ASSERT(x + x0 < original.width());
                    BOOST_ASSERT(y + y0 < original.height());

                    for( int c = 0; c < 4; ++c )
                    {
                        const auto pixel = srcImage(x, y, 0, c) / 255.0f;
                        BOOST_ASSERT(pixel >= 0 && pixel <= 1);
                        original(x + x0, y + y0, 0, c) = pixel;
                    }
                }
            }
        }

        BOOST_LOG_TRIVIAL(info) << "Writing texture cache " << cacheName << "...";
        boost::filesystem::create_directories(cacheName.parent_path());
        (original * 255).save_png(cacheName.string().c_str(), 1);

        // interleave
        original.permute_axes("cxyz");

        return std::make_shared<gameplay::Image>(Resolution, Resolution, reinterpret_cast<const glm::vec4*>(original.data()));
    }


    std::shared_ptr<gameplay::Texture> DWordTexture::toTexture(trx::Glidos* glidos, const boost::filesystem::path& lvlName) const
    {
        return std::make_shared<gameplay::Texture>(toImage(glidos, lvlName), true);
    }
}
