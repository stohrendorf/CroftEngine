#pragma once

#include "Base.h"
#include "gl/util.h"

#include <boost/optional.hpp>

#include <map>
#include <memory>
#include <vector>

namespace gameplay
{
class RenderState
{
public:
    RenderState(const RenderState&) = delete;

    RenderState& operator=(const RenderState&) = delete;

    explicit RenderState() = default;

    virtual ~RenderState() = default;

    void bind();

    void setBlend(bool enabled);

    void setBlendSrc(GLenum blend);

    void setBlendDst(GLenum blend);

    void setCullFace(bool enabled);

    void setCullFaceSide(GLenum side);

    void setFrontFace(GLenum winding);

    void setDepthTest(bool enabled);

    void setDepthWrite(bool enabled);

    void setDepthFunction(GLenum func);

    static void initDefaults()
    {
        m_currentState.restore( true );
    }

    static void enableDepthWrite();

private:
    void bindNoRestore();

    void restore(bool forceDefaults = false);

    // States
    boost::optional<bool> m_cullFaceEnabled;

    boost::optional<bool> m_depthTestEnabled;

    boost::optional<bool> m_depthWriteEnabled;

    boost::optional<GLenum> m_depthFunction;

    boost::optional<bool> m_blendEnabled;

    boost::optional<GLenum> m_blendSrc;

    boost::optional<GLenum> m_blendDst;

    boost::optional<GLenum> m_cullFaceSide;

    boost::optional<GLenum> m_frontFace;

    static RenderState m_currentState;
};
}
