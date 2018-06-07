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


    template<typename T, const T DefaultValue>
    struct DefaultedOptional
    {
        static const constexpr T Default = DefaultValue;
        boost::optional<T> value{};

        bool isDefault() const
        {
            return get() == Default;
        }

        constexpr T getDefault() const noexcept
        {
            return Default;
        }

        T get() const
        {
            return value.get_value_or( Default );
        }

        void reset()
        {
            value.reset();
        }

        bool isInitialized() const
        {
            return value.is_initialized();
        }

        bool operator!=(const DefaultedOptional<T, DefaultValue>& rhs)
        {
            return value != rhs.value;
        }

        DefaultedOptional<T, DefaultValue>& operator=(T rhs)
        {
            if( rhs == Default )
                value.reset();
            else
                value = rhs;
            return *this;
        }
    };


    // States
    DefaultedOptional<bool, true> m_cullFaceEnabled;

    DefaultedOptional<bool, true> m_depthTestEnabled;

    DefaultedOptional<bool, true> m_depthWriteEnabled;

    DefaultedOptional<GLenum, GL_LESS> m_depthFunction;

    DefaultedOptional<bool, true> m_blendEnabled;

    DefaultedOptional<GLenum, GL_SRC_ALPHA> m_blendSrc;

    DefaultedOptional<GLenum, GL_ONE_MINUS_SRC_ALPHA> m_blendDst;

    DefaultedOptional<GLenum, GL_BACK> m_cullFaceSide;

    DefaultedOptional<GLenum, GL_CW> m_frontFace;

    static RenderState m_currentState;
};
}
