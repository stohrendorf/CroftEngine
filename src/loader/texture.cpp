#include "texture.h"

#include "engine/items/itemnode.h"


namespace loader
{
    std::shared_ptr<gameplay::Material> createMaterial(const std::shared_ptr<gameplay::Texture>& texture, BlendingMode bmode, const std::shared_ptr<gameplay::ShaderProgram>& shader)
    {
        auto result = std::make_shared<gameplay::Material>(shader);
        // Set some defaults
        auto sampler = std::make_shared<gameplay::Texture::Sampler>(texture);
        sampler->setWrapMode(gameplay::Texture::CLAMP, gameplay::Texture::CLAMP);
        result->getParameter("u_diffuseTexture")->set(sampler);
        result->getParameter("u_worldViewProjectionMatrix")->bindWorldViewProjectionMatrix();
        result->getParameter("u_modelMatrix")->bindModelMatrix();
        result->getParameter("u_viewMatrix")->bindViewMatrix();
        result->initStateBlockDefaults();

        result->getParameter("u_brightness")->bind(&engine::items::ItemNode::lightBrightnessBinder);
        result->getParameter("u_lightPosition")->bind(&engine::items::ItemNode::lightPositionBinder);

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
}
