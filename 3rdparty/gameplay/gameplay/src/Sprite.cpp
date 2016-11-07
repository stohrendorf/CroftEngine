#include "Base.h"
#include "Sprite.h"
#include "Scene.h"

#include <boost/log/trivial.hpp>

namespace gameplay
{
    Sprite::Sprite(Game* game, const std::shared_ptr<Texture>& texture, float width, float height, const Rectangle& source, unsigned frameCount, const std::shared_ptr<ShaderProgram>& shaderProgram)
        : Drawable()
        , _width{width}
        , _height{height}
        , _offset(OFFSET_BOTTOM_LEFT)
        , _anchor(glm::vec2(0.5f, 0.5f))
        , _flipFlags(FLIP_NONE)
        , _frames(frameCount)
        , _frameStride(0)
        , _framePadding(1)
        , _frameIndex(0)
        , _batch{ std::make_shared<SpriteBatch>(game, texture, shaderProgram) }
        , _opacity(1.0f)
        , _color{ 1,1,1,1 }
        , _blendMode(BLEND_ALPHA)
    {
        BOOST_ASSERT(texture != nullptr);
        BOOST_ASSERT(width >= -1 && height >= -1);
        BOOST_ASSERT(source.width >= -1 && source.height >= -1);
        BOOST_ASSERT(frameCount > 0);

        _batch->getSampler()->setWrapMode(Texture::CLAMP, Texture::CLAMP);
        _batch->getSampler()->setFilterMode(Texture::Filter::LINEAR, Texture::Filter::LINEAR);
        _batch->getStateBlock()->setDepthWrite(false);
        _batch->getStateBlock()->setDepthTest(true);

        auto imageWidth = _batch->getSampler()->getTexture()->getWidth();
        auto imageHeight = _batch->getSampler()->getTexture()->getHeight();
        if(width == -1)
            _width = imageWidth;
        if(height == -1)
            _height = imageHeight;

        _frames[0] = source;
        if(_frames[0].width == -1.0f)
            _frames[0].width = imageWidth;
        if(_frames[0].height == -1.0f)
            _frames[0].height = imageHeight;
    }


    Sprite::~Sprite() = default;


    float Sprite::getWidth() const
    {
        return _width;
    }


    float Sprite::getHeight() const
    {
        return _height;
    }


    void Sprite::setOffset(Sprite::Offset offset)
    {
        _offset = offset;
    }


    Sprite::Offset Sprite::getOffset() const
    {
        return _offset;
    }


    void Sprite::setAnchor(const glm::vec2& anchor)
    {
        _anchor = anchor;
    }


    const glm::vec2& Sprite::getAnchor() const
    {
        return _anchor;
    }


    void Sprite::setFlip(int flipFlags)
    {
        _flipFlags = flipFlags;
    }


    int Sprite::getFlip() const
    {
        return _flipFlags;
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void Sprite::setFrameSource(size_t frameIndex, const Rectangle& source)
    {
        BOOST_ASSERT(frameIndex < _frames.size());

        _frames[frameIndex] = source;
    }


    const Rectangle& Sprite::getFrameSource(size_t frameIndex) const
    {
        BOOST_ASSERT(frameIndex < _frames.size());

        return _frames[frameIndex];
    }


    void Sprite::computeFrames(unsigned int frameStride, unsigned int framePadding)
    {
        _frameStride = frameStride;
        _framePadding = framePadding;

        if( _frames.size() < 2 )
            return;
        unsigned int imageWidth = _batch->getSampler()->getTexture()->getWidth();
        unsigned int imageHeight = _batch->getSampler()->getTexture()->getHeight();

        // Mark the start as reference
        float x = _frames[0].x;
        float y = _frames[0].y;

        // Compute frames 1+
        for( unsigned int frameIndex = 1; frameIndex < _frames.size(); frameIndex++ )
        {
            _frames[frameIndex].x = x;
            _frames[frameIndex].y = y;
            _frames[frameIndex].width = _width;
            _frames[frameIndex].height = _height;

            x += _frames[frameIndex].width + (float)_framePadding;
            if( x >= imageWidth )
            {
                y += _frames[frameIndex].height + (float)_framePadding;
                if( y >= imageHeight )
                {
                    y = 0.0f;
                }
                x = 0.0f;
            }
        }
    }


    size_t Sprite::getFrameCount() const
    {
        return _frames.size();
    }


    unsigned int Sprite::getFramePadding() const
    {
        return _framePadding;
    }


    unsigned int Sprite::getFrameStride() const
    {
        return _frameStride;
    }


    void Sprite::setFrameIndex(unsigned int index)
    {
        _frameIndex = index;
    }


    unsigned int Sprite::getFrameIndex() const
    {
        return _frameIndex;
    }


    void Sprite::setOpacity(float opacity)
    {
        _opacity = opacity;
    }


    float Sprite::getOpacity() const
    {
        return _opacity;
    }


    void Sprite::setColor(const glm::vec4& color)
    {
        _color = color;
    }


    const glm::vec4& Sprite::getColor() const
    {
        return _color;
    }


    Sprite::BlendMode Sprite::getBlendMode() const
    {
        return _blendMode;
    }


    void Sprite::setBlendMode(BlendMode mode)
    {
        switch( mode )
        {
            case BLEND_NONE:
                _batch->getStateBlock()->setBlend(false);
                break;
            case BLEND_ALPHA:
                _batch->getStateBlock()->setBlend(true);
                _batch->getStateBlock()->setBlendSrc(RenderState::BLEND_SRC_ALPHA);
                _batch->getStateBlock()->setBlendDst(RenderState::BLEND_ONE_MINUS_SRC_ALPHA);
                break;
            case BLEND_ADDITIVE:
                _batch->getStateBlock()->setBlend(true);
                _batch->getStateBlock()->setBlendSrc(RenderState::BLEND_SRC_ALPHA);
                _batch->getStateBlock()->setBlendDst(RenderState::BLEND_ONE);
                break;
            case BLEND_MULTIPLIED:
                _batch->getStateBlock()->setBlend(true);
                _batch->getStateBlock()->setBlendSrc(RenderState::BLEND_ZERO);
                _batch->getStateBlock()->setBlendDst(RenderState::BLEND_SRC_COLOR);
                break;
            default:
                BOOST_LOG_TRIVIAL(error) << "Unsupported blend mode (" << mode << ").";
                break;
        }
    }


    const std::shared_ptr<Texture::Sampler>& Sprite::getSampler() const
    {
        return _batch->getSampler();
    }


    std::shared_ptr<RenderState::StateBlock> Sprite::getStateBlock() const
    {
        return _batch->getStateBlock();
    }


    const std::shared_ptr<Material>& Sprite::getMaterial() const
    {
        return _batch->getMaterial();
    }


    size_t Sprite::draw(bool /*wireframe*/)
    {
        // Apply scene camera projection and translation offsets
        glm::vec3 position{ 0,0,0 };
        if( _node && _node->getScene() )
        {
            auto activeCamera = _node->getScene()->getActiveCamera();
            if( activeCamera )
            {
                auto cameraNode = _node->getScene()->getActiveCamera()->getNode();
                if( cameraNode )
                {
                    // Scene projection
                    glm::mat4 projectionMatrix;
                    projectionMatrix = _node->getProjectionMatrix();
                    _batch->setProjectionMatrix(projectionMatrix);

                    // Camera translation offsets
                    position.x -= cameraNode->getTranslationWorld().x;
                    position.y -= cameraNode->getTranslationWorld().y;
                }
            }

            // Apply node translation offsets
            glm::vec3 translation = _node->getTranslationWorld();
            position.x += translation.x;
            position.y += translation.y;
            position.z += translation.z;
        }

        // Apply local offset translation offsets
        if( (_offset & OFFSET_HCENTER) == OFFSET_HCENTER )
            position.x -= _width * 0.5f;
        if( (_offset & OFFSET_RIGHT) == OFFSET_RIGHT )
            position.x -= _width;
        if( (_offset & OFFSET_VCENTER) == OFFSET_VCENTER )
            position.y -= _height * 0.5f;
        if( (_offset & OFFSET_TOP) == OFFSET_TOP )
            position.y -= _height;
        if( (_offset & OFFSET_ANCHOR) == OFFSET_ANCHOR )
        {
            position.x -= _width * _anchor.x;
            position.y -= _height * _anchor.y;
        }

        // Apply node scale and rotation
        float rotationAngle = 0.0f;
        glm::vec2 scale = glm::vec2(_width, _height);
        if( _node )
        {
            // Apply node rotation
            const glm::quat& rot = _node->getRotation();
            if(rot.x != 0.0f || rot.y != 0.0f || rot.z != 0.0f)
                rotationAngle = glm::angle(rot);

            // Apply node scale
            if( _node->getScaleX() != 1.0f )
                scale.x *= _node->getScaleX();
            if( _node->getScaleY() != 1.0f )
                scale.y *= _node->getScaleY();
        }

        // Apply flip flags
        if( (_flipFlags & FLIP_HORIZONTAL) == FLIP_HORIZONTAL )
        {
            position.x += scale.x;
            scale.x = -scale.x;
        }
        if( (_flipFlags & FLIP_VERTICAL) == FLIP_VERTICAL )
        {
            position.y += scale.y;
            scale.y = -scale.y;
        }

        // TODO: Proper batching from cache based on batching rules (image, layers, etc)
        _batch->start();
        _batch->draw(position, _frames[_frameIndex], scale, glm::vec4(_color.x, _color.y, _color.z, _color.w * _opacity),
                     _anchor, rotationAngle);
        _batch->finishAndDraw();

        return 1;
    }
}
