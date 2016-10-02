#include "Base.h"
#include "SpriteBatch.h"
#include "Game.h"
#include "Material.h"
#include "MaterialParameter.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <boost/log/trivial.hpp>

// Default size of a newly created sprite batch
#define SPRITE_BATCH_DEFAULT_SIZE 128

// Factor to grow a sprite batch by when its size is exceeded
#define SPRITE_BATCH_GROW_FACTOR 2.0f

// Macro for adding a sprite to the batch
#define SPRITE_ADD_VERTEX(vtx, vx, vy, vz, vu, vv, vr, vg, vb, va) \
    vtx.x = vx; vtx.y = vy; vtx.z = vz; \
    vtx.u = vu; vtx.v = vv; \
    vtx.r = vr; vtx.g = vg; vtx.b = vb; vtx.a = va

// Default sprite shaders
#define SPRITE_VSH "shaders/sprite.vert"
#define SPRITE_FSH "shaders/sprite.frag"


namespace gameplay
{
    static std::shared_ptr<ShaderProgram> __spriteShaderProgram = nullptr;


    SpriteBatch::SpriteBatch()
        : _batch(nullptr)
        , _sampler(nullptr)
        , _customEffect(false)
        , _textureWidthRatio(0.0f)
        , _textureHeightRatio(0.0f)
    {
    }


    SpriteBatch::~SpriteBatch()
    {
        SAFE_DELETE(_batch);
    }


    SpriteBatch* SpriteBatch::create(const std::shared_ptr<Texture>& texture, const std::shared_ptr<ShaderProgram>& shaderProgram, unsigned int initialCapacity)
    {
        BOOST_ASSERT(texture != nullptr);
        BOOST_ASSERT(texture->getType() == Texture::TEXTURE_2D);

        auto fx = shaderProgram;
        bool customEffect = (fx != nullptr);
        if( !customEffect )
        {
            // Create our static sprite effect.
            if( __spriteShaderProgram == nullptr )
            {
                __spriteShaderProgram = ShaderProgram::createFromFile(SPRITE_VSH, SPRITE_FSH);
                if( __spriteShaderProgram == nullptr )
                {
                    BOOST_LOG_TRIVIAL(error) << "Unable to load sprite effect.";
                    return nullptr;
                }
                fx = __spriteShaderProgram;
            }
            else
            {
                fx = __spriteShaderProgram;
            }
        }

        // Search for the first sampler uniform in the effect.
        std::shared_ptr<Uniform> samplerUniform = nullptr;
        for( unsigned int i = 0, count = fx->getUniformCount(); i < count; ++i )
        {
            auto uniform = fx->getUniform(i);
            if( uniform && uniform->getType() == GL_SAMPLER_2D )
            {
                samplerUniform = uniform;
                break;
            }
        }
        if( !samplerUniform )
        {
            BOOST_LOG_TRIVIAL(error) << "No uniform of type GL_SAMPLER_2D found in sprite effect.";
            return nullptr;
        }

        // Wrap the effect in a material
        auto material = Material::create(fx);

        // Set initial material state
        material->getStateBlock()->setBlend(true);
        material->getStateBlock()->setBlendSrc(RenderState::BLEND_SRC_ALPHA);
        material->getStateBlock()->setBlendDst(RenderState::BLEND_ONE_MINUS_SRC_ALPHA);

        // Bind the texture to the material as a sampler
        auto sampler = Texture::Sampler::create(texture);
        material->getParameter(samplerUniform->getName())->setValue(sampler);

        // Define the vertex format for the batch
        VertexFormat::Element vertexElements[] =
        {
            VertexFormat::Element(VertexFormat::POSITION, 3),
            VertexFormat::Element(VertexFormat::TEXCOORD0, 2),
            VertexFormat::Element(VertexFormat::COLOR, 4)
        };
        VertexFormat vertexFormat(vertexElements, 3);

        // Create the mesh batch
        MeshBatch* meshBatch = new MeshBatch(vertexFormat, Mesh::TRIANGLE_STRIP, material, true, initialCapacity > 0 ? initialCapacity : SPRITE_BATCH_DEFAULT_SIZE);

        // Create the batch
        SpriteBatch* batch = new SpriteBatch();
        batch->_sampler = sampler;
        batch->_customEffect = customEffect;
        batch->_batch = meshBatch;
        batch->_textureWidthRatio = 1.0f / (float)texture->getWidth();
        batch->_textureHeightRatio = 1.0f / (float)texture->getHeight();

        // Bind an ortho projection to the material by default (user can override with setProjectionMatrix)
        Game* game = Game::getInstance();
        batch->_projectionMatrix = glm::ortho(0.0f, game->getViewport().width, game->getViewport().height, 0.0f, 0.0f, 1.0f);
        material->getParameter("u_projectionMatrix")->bindValue(batch, &SpriteBatch::getProjectionMatrix);

        return batch;
    }


    void SpriteBatch::start()
    {
        _batch->start();
    }


    bool SpriteBatch::isStarted() const
    {
        return _batch->isStarted();
    }


    void SpriteBatch::draw(const Rectangle& dst, const Rectangle& src, const glm::vec4& color)
    {
        // Calculate uvs.
        float u1 = _textureWidthRatio * src.x;
        float v1 = 1.0f - _textureHeightRatio * src.y;
        float u2 = u1 + _textureWidthRatio * src.width;
        float v2 = v1 - _textureHeightRatio * src.height;

        draw(dst.x, dst.y, dst.width, dst.height, u1, v1, u2, v2, color);
    }


    void SpriteBatch::draw(const glm::vec3& dst, const Rectangle& src, const glm::vec2& scale, const glm::vec4& color)
    {
        // Calculate uvs.
        float u1 = _textureWidthRatio * src.x;
        float v1 = 1.0f - _textureHeightRatio * src.y;
        float u2 = u1 + _textureWidthRatio * src.width;
        float v2 = v1 - _textureHeightRatio * src.height;

        draw(dst.x, dst.y, dst.z, scale.x, scale.y, u1, v1, u2, v2, color);
    }


    void SpriteBatch::draw(const glm::vec3& dst, const Rectangle& src, const glm::vec2& scale, const glm::vec4& color,
                           const glm::vec2& rotationPoint, float rotationAngle)
    {
        // Calculate uvs.
        float u1 = _textureWidthRatio * src.x;
        float v1 = 1.0f - _textureHeightRatio * src.y;
        float u2 = u1 + _textureWidthRatio * src.width;
        float v2 = v1 - _textureHeightRatio * src.height;

        draw(dst, scale.x, scale.y, u1, v1, u2, v2, color, rotationPoint, rotationAngle);
    }


    void SpriteBatch::draw(const glm::vec3& dst, float width, float height, float u1, float v1, float u2, float v2, const glm::vec4& color,
                           const glm::vec2& rotationPoint, float rotationAngle, bool positionIsCenter)
    {
        draw(dst.x, dst.y, dst.z, width, height, u1, v1, u2, v2, color, rotationPoint, rotationAngle, positionIsCenter);
    }


    void SpriteBatch::draw(float x, float y, float z, float width, float height, float u1, float v1, float u2, float v2, const glm::vec4& color,
                           const glm::vec2& rotationPoint, float rotationAngle, bool positionIsCenter)
    {
        // Treat the given position as the center if the user specified it as such.
        if( positionIsCenter )
        {
            x -= 0.5f * width;
            y -= 0.5f * height;
        }

        // Expand the destination position by scale into 4 points.
        float x2 = x + width;
        float y2 = y + height;

        glm::vec2 upLeft(x, y);
        glm::vec2 upRight(x2, y);
        glm::vec2 downLeft(x, y2);
        glm::vec2 downRight(x2, y2);

        // Rotate points around rotationAxis by rotationAngle.
        if( rotationAngle != 0 )
        {
            glm::vec2 pivotPoint(rotationPoint);
            pivotPoint.x *= width;
            pivotPoint.y *= height;
            pivotPoint.x += x;
            pivotPoint.y += y;

            auto rotate = [&pivotPoint, &rotationAngle](glm::vec2& vec) -> void
            {
                const auto sinAngle = glm::sin(rotationAngle);
                const auto cosAngle = glm::cos(rotationAngle);

                const auto tempX = vec.x - pivotPoint.x;
                const auto tempY = vec.y - pivotPoint.y;

                vec.x = tempX * cosAngle - tempY * sinAngle + pivotPoint.x;
                vec.y = tempY * cosAngle + tempX * sinAngle + pivotPoint.y;
            };

            rotate(upLeft);
            rotate(upRight);
            rotate(downLeft);
            rotate(downRight);
        }

        // Write sprite vertex data.
        static SpriteVertex v[4];
        SPRITE_ADD_VERTEX(v[0], downLeft.x, downLeft.y, z, u1, v1, color.x, color.y, color.z, color.w);
        SPRITE_ADD_VERTEX(v[1], upLeft.x, upLeft.y, z, u1, v2, color.x, color.y, color.z, color.w);
        SPRITE_ADD_VERTEX(v[2], downRight.x, downRight.y, z, u2, v1, color.x, color.y, color.z, color.w);
        SPRITE_ADD_VERTEX(v[3], upRight.x, upRight.y, z, u2, v2, color.x, color.y, color.z, color.w);

        static unsigned short indices[4] = {0, 1, 2, 3};

        _batch->add(v, 4, indices, 4);
    }


    void SpriteBatch::draw(const glm::vec3& position, const glm::vec3& right, const glm::vec3& forward, float width, float height,
                           float u1, float v1, float u2, float v2, const glm::vec4& color, const glm::vec2& rotationPoint, float rotationAngle)
    {
        // Calculate the vertex positions.
        glm::vec3 tRight(right);
        tRight *= width * 0.5f;
        glm::vec3 tForward(forward);
        tForward *= height * 0.5f;

        glm::vec3 p0 = position;
        p0 -= tRight;
        p0 -= tForward;

        glm::vec3 p1 = position;
        p1 += tRight;
        p1 -= tForward;

        tForward = forward;
        tForward *= height;
        glm::vec3 p2 = p0;
        p2 += tForward;
        glm::vec3 p3 = p1;
        p3 += tForward;

        // Calculate the rotation point.
        if( rotationAngle != 0 )
        {
            glm::vec3 rp = p0;
            tRight = right;
            tRight *= width * rotationPoint.x;
            tForward *= rotationPoint.y;
            rp += tRight;
            rp += tForward;

            // Rotate all points the specified amount about the given point (about the up vector).
            const auto rotation = glm::angleAxis(rotationAngle, glm::cross(right, forward));
            p0 = rotation * (p0 - rp) + rp;
            p1 = rotation * (p1 - rp) + rp;
            p2 = rotation * (p2 - rp) + rp;
            p3 = rotation * (p3 - rp) + rp;
        }

        // Add the sprite vertex data to the batch.
        static SpriteVertex v[4];
        SPRITE_ADD_VERTEX(v[0], p0.x, p0.y, p0.z, u1, v1, color.x, color.y, color.z, color.w);
        SPRITE_ADD_VERTEX(v[1], p1.x, p1.y, p1.z, u2, v1, color.x, color.y, color.z, color.w);
        SPRITE_ADD_VERTEX(v[2], p2.x, p2.y, p2.z, u1, v2, color.x, color.y, color.z, color.w);
        SPRITE_ADD_VERTEX(v[3], p3.x, p3.y, p3.z, u2, v2, color.x, color.y, color.z, color.w);

        static const unsigned short indices[4] = {0, 1, 2, 3};
        _batch->add(v, 4, const_cast<unsigned short*>(indices), 4);
    }


    void SpriteBatch::draw(float x, float y, float width, float height, float u1, float v1, float u2, float v2, const glm::vec4& color)
    {
        draw(x, y, 0, width, height, u1, v1, u2, v2, color);
    }


    void SpriteBatch::draw(float x, float y, float width, float height, float u1, float v1, float u2, float v2, const glm::vec4& color, const Rectangle& clip)
    {
        draw(x, y, 0, width, height, u1, v1, u2, v2, color, clip);
    }


    void SpriteBatch::draw(float x, float y, float z, float width, float height, float u1, float v1, float u2, float v2, const glm::vec4& color, const Rectangle& clip)
    {
        // TODO: Perform software clipping instead of culling the entire sprite.

        // Only draw if at least part of the sprite is within the clip region.
        if( clipSprite(clip, x, y, width, height, u1, v1, u2, v2) )
            draw(x, y, z, width, height, u1, v1, u2, v2, color);
    }


    void SpriteBatch::addSprite(float x, float y, float width, float height, float u1, float v1, float u2, float v2, const glm::vec4& color, SpriteBatch::SpriteVertex* vertices)
    {
        BOOST_ASSERT(vertices);

        const float x2 = x + width;
        const float y2 = y + height;
        SPRITE_ADD_VERTEX(vertices[0], x, y, 0, u1, v1, color.x, color.y, color.z, color.w);
        SPRITE_ADD_VERTEX(vertices[1], x, y2, 0, u1, v2, color.x, color.y, color.z, color.w);
        SPRITE_ADD_VERTEX(vertices[2], x2, y, 0, u2, v1, color.x, color.y, color.z, color.w);
        SPRITE_ADD_VERTEX(vertices[3], x2, y2, 0, u2, v2, color.x, color.y, color.z, color.w);
    }


    void SpriteBatch::addSprite(float x, float y, float width, float height, float u1, float v1, float u2, float v2, const glm::vec4& color, const Rectangle& clip, SpriteBatch::SpriteVertex* vertices)
    {
        BOOST_ASSERT(vertices);

        // Only add a sprite if at least part of the sprite is within the clip region.
        if( clipSprite(clip, x, y, width, height, u1, v1, u2, v2) )
        {
            const float x2 = x + width;
            const float y2 = y + height;
            SPRITE_ADD_VERTEX(vertices[0], x, y, 0, u1, v1, color.x, color.y, color.z, color.w);
            SPRITE_ADD_VERTEX(vertices[1], x, y2, 0, u1, v2, color.x, color.y, color.z, color.w);
            SPRITE_ADD_VERTEX(vertices[2], x2, y, 0, u2, v1, color.x, color.y, color.z, color.w);
            SPRITE_ADD_VERTEX(vertices[3], x2, y2, 0, u2, v2, color.x, color.y, color.z, color.w);
        }
    }


    void SpriteBatch::draw(SpriteBatch::SpriteVertex* vertices, unsigned int vertexCount, unsigned short* indices, unsigned int indexCount) const
    {
        BOOST_ASSERT(vertices);
        BOOST_ASSERT(indices);

        _batch->add(vertices, vertexCount, indices, indexCount);
    }


    void SpriteBatch::draw(float x, float y, float z, float width, float height, float u1, float v1, float u2, float v2, const glm::vec4& color, bool positionIsCenter) const
    {
        // Treat the given position as the center if the user specified it as such.
        if( positionIsCenter )
        {
            x -= 0.5f * width;
            y -= 0.5f * height;
        }

        // Write sprite vertex data.
        const float x2 = x + width;
        const float y2 = y + height;
        static SpriteVertex v[4];
        SPRITE_ADD_VERTEX(v[0], x, y, z, u1, v1, color.x, color.y, color.z, color.w);
        SPRITE_ADD_VERTEX(v[1], x, y2, z, u1, v2, color.x, color.y, color.z, color.w);
        SPRITE_ADD_VERTEX(v[2], x2, y, z, u2, v1, color.x, color.y, color.z, color.w);
        SPRITE_ADD_VERTEX(v[3], x2, y2, z, u2, v2, color.x, color.y, color.z, color.w);

        static unsigned short indices[4] = {0, 1, 2, 3};

        _batch->add(v, 4, indices, 4);
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void SpriteBatch::finish()
    {
        // Finish and draw the batch
        _batch->finish();
        _batch->draw();
    }


    std::shared_ptr<RenderState::StateBlock> SpriteBatch::getStateBlock() const
    {
        return _batch->getMaterial()->getStateBlock();
    }


    const std::shared_ptr<Texture::Sampler>& SpriteBatch::getSampler() const
    {
        return _sampler;
    }


    const std::shared_ptr<Material>& SpriteBatch::getMaterial() const
    {
        return _batch->getMaterial();
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void SpriteBatch::setProjectionMatrix(const glm::mat4& matrix)
    {
        _projectionMatrix = matrix;
    }


    const glm::mat4& SpriteBatch::getProjectionMatrix() const
    {
        return _projectionMatrix;
    }


    bool SpriteBatch::clipSprite(const Rectangle& clip, float& x, float& y, float& width, float& height, float& u1, float& v1, float& u2, float& v2)
    {
        // Clip the rectangle given by { x, y, width, height } into clip.
        // We need to scale the uvs accordingly as we do this.

        // First check to see if we need to draw at all.
        if( x + width < clip.x || x > clip.x + clip.width ||
                                  y + height < clip.y || y > clip.y + clip.height )
        {
            return false;
        }

        float uvWidth = u2 - u1;
        float uvHeight = v2 - v1;

        // Moving x to the right.
        if( x < clip.x )
        {
            const float percent = (clip.x - x) / width;
            const float dx = clip.x - x;
            const float du = uvWidth * percent;
            x = clip.x;
            width -= dx;
            u1 += du;
            uvWidth -= du;
        }

        // Moving y down.
        if( y < clip.y )
        {
            const float percent = (clip.y - y) / height;
            const float dy = clip.y - y;
            const float dv = uvHeight * percent;
            y = clip.y;
            height -= dy;
            v1 += dv;
            uvHeight -= dv;
        }

        // Moving width to the left.
        const float clipX2 = clip.x + clip.width;
        float x2 = x + width;
        if( x2 > clipX2 )
        {
            const float percent = (x2 - clipX2) / width;
            width = clipX2 - x;
            u2 -= uvWidth * percent;
        }

        // Moving height up.
        const float clipY2 = clip.y + clip.height;
        float y2 = y + height;
        if( y2 > clipY2 )
        {
            const float percent = (y2 - clipY2) / height;
            height = clipY2 - y;
            v2 -= uvHeight * percent;
        }

        return true;
    }
}
