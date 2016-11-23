#include "Base.h"
#include "RenderTarget.h"

namespace gameplay
{
    RenderTarget::~RenderTarget() = default;


    RenderTarget::RenderTarget(unsigned int width, unsigned int height)
        : _texture{std::make_shared<Texture>(width, height, std::vector<glm::vec4>(), false)}
    {
    }


    RenderTarget::RenderTarget(const std::shared_ptr<Texture>& texture)
        : _texture{texture}
    {
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
