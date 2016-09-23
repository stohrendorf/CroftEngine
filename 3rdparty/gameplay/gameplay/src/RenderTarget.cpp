#include "Base.h"
#include "RenderTarget.h"


namespace gameplay
{
    static std::vector<std::shared_ptr<RenderTarget>> __renderTargets;


    RenderTarget::RenderTarget(const char* id)
        : _id(id ? id : "")
        , _texture(nullptr)
    {
    }


    RenderTarget::~RenderTarget()
    {
        // Remove ourself from the cache.
        auto it = std::find(__renderTargets.begin(), __renderTargets.end(), shared_from_this());
        if( it != __renderTargets.end() )
        {
            __renderTargets.erase(it);
        }
    }


    std::shared_ptr<RenderTarget> RenderTarget::create(const char* id, unsigned int width, unsigned int height)
    {
        // Create a new texture with the given width.
        auto texture = Texture::create(width, height, {}, false);
        if( texture == nullptr )
        {
            GP_ERROR("Failed to create texture for render target.");
            return nullptr;
        }

        return create(id, texture);
    }


    std::shared_ptr<RenderTarget> RenderTarget::create(const char* id, const std::shared_ptr<Texture>& texture)
    {
        auto renderTarget = std::make_shared<RenderTarget>(id);
        renderTarget->_texture = texture;

        __renderTargets.push_back(renderTarget);

        return renderTarget;
    }


    std::shared_ptr<RenderTarget> RenderTarget::getRenderTarget(const char* id)
    {
        BOOST_ASSERT(id);

        // Search the vector for a matching ID.
        for( auto it = __renderTargets.begin(); it < __renderTargets.end(); ++it )
        {
            auto dst = *it;
            BOOST_ASSERT(dst);
            if( strcmp(id, dst->getId()) == 0 )
            {
                return dst;
            }
        }

        return nullptr;
    }


    const char* RenderTarget::getId() const
    {
        return _id.c_str();
    }


    const std::shared_ptr<Texture>& RenderTarget::getTexture() const
    {
        return _texture;
    }


    unsigned int RenderTarget::getWidth() const
    {
        return _texture->getWidth();
    }


    unsigned int RenderTarget::getHeight() const
    {
        return _texture->getHeight();
    }
}
