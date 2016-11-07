#include "Base.h"
#include "RenderTarget.h"

#include <boost/log/trivial.hpp>


namespace gameplay
{
    static std::vector<std::shared_ptr<RenderTarget>> __renderTargets;


    RenderTarget::RenderTarget(const std::string& id)
        : _id(id)
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


    std::shared_ptr<RenderTarget> RenderTarget::create(const std::string& id, unsigned int width, unsigned int height)
    {
        // Create a new texture with the given width.
        auto texture = std::make_shared<Texture>(width, height, std::vector<glm::vec4>(), false);

        return create(id, texture);
    }


    std::shared_ptr<RenderTarget> RenderTarget::create(const std::string& id, const std::shared_ptr<Texture>& texture)
    {
        auto renderTarget = std::make_shared<RenderTarget>(id);
        renderTarget->_texture = texture;

        __renderTargets.push_back(renderTarget);

        return renderTarget;
    }


    std::shared_ptr<RenderTarget> RenderTarget::getRenderTarget(const std::string& id)
    {
        // Search the vector for a matching ID.
        for( auto it = __renderTargets.begin(); it < __renderTargets.end(); ++it )
        {
            auto dst = *it;
            BOOST_ASSERT(dst);
            if( id == dst->getId() )
            {
                return dst;
            }
        }

        return nullptr;
    }


    const std::string& RenderTarget::getId() const
    {
        return _id;
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
