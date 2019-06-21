#include "renderer.h"

#include "Scene.h"
#include "names.h"
#include "render/gl/debuggroup.h"
#include "rendercontext.h"

namespace render
{
namespace scene
{
Renderer::Renderer()
    : m_scene{std::make_shared<Scene>()}
{
}

Renderer::~Renderer() = default;

namespace
{
class RenderVisitor : public Visitor
{
public:
    explicit RenderVisitor(RenderContext& context)
        : Visitor{context}
    {
    }

    void visit(Node& node) override
    {
        if(!node.isVisible())
        {
            return;
        }

        gl::DebugGroup debugGroup{node.getId()};

        getContext().setCurrentNode(&node);

        if(auto dr = node.getDrawable())
        {
            dr->render(getContext());
        }

        Visitor::visit(node);
    }
};
} // namespace

void Renderer::render()
{
    // Graphics Rendering.
    clear(::gl::ClearBufferMask::ColorBufferBit | ::gl::ClearBufferMask::DepthBufferBit, {0, 0, 0, 0}, 1);

    RenderContext context{};
    RenderVisitor visitor{context};
    m_scene->accept(visitor);

    // Update FPS.
    ++m_frameCount;
    const auto t = getGameTime();
    const auto dt = t - m_frameLastFPS;
    if(dt >= std::chrono::seconds(1))
    {
        m_frameRate = std::exchange(m_frameCount, 0) * 1000.0f
                      / std::chrono::duration_cast<std::chrono::milliseconds>(dt).count();
        m_frameLastFPS = t;
    }
}

void Renderer::clear(const ::gl::core::Bitfield<::gl::ClearBufferMask> flags,
                     const gl::SRGBA8& clearColor,
                     const float clearDepth)
{
    ::gl::core::Bitfield<::gl::ClearBufferMask> bits;
    if(flags.isSet(::gl::ClearBufferMask::ColorBufferBit))
    {
        if(clearColor != m_clearColor)
        {
            GL_ASSERT(::gl::clearColor(
                clearColor.r / 255.0f, clearColor.g / 255.0f, clearColor.b / 255.0f, clearColor.a / 255.0f));
            m_clearColor = clearColor;
        }
        bits |= ::gl::ClearBufferMask::ColorBufferBit;
    }

    if(flags.isSet(::gl::ClearBufferMask::DepthBufferBit))
    {
        if(clearDepth != m_clearDepth)
        {
            GL_ASSERT(::gl::clearDepth(clearDepth));
            m_clearDepth = clearDepth;
        }
        bits |= ::gl::ClearBufferMask::DepthBufferBit;

        // We need to explicitly call the static enableDepthWrite() method on StateBlock
        // to ensure depth writing is enabled before clearing the depth buffer (and to
        // update the global StateBlock render state to reflect this).
        render::gl::RenderState::enableDepthWrite();
    }

    GL_ASSERT(::gl::clear(bits));
}

void Renderer::clear(const ::gl::core::Bitfield<::gl::ClearBufferMask> flags,
                     const uint8_t red,
                     const uint8_t green,
                     const uint8_t blue,
                     const uint8_t alpha,
                     const float clearDepth)
{
    clear(flags, gl::SRGBA8{red, green, blue, alpha}, clearDepth);
}
} // namespace scene
} // namespace render
