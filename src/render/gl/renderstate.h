#pragma once

#include "glassert.h"

#include <boost/optional.hpp>

namespace render
{
namespace gl
{
enum class CullFaceMode : RawGlEnum
{
    Back = (RawGlEnum)::gl::GL_BACK,
    Front = (RawGlEnum)::gl::GL_FRONT,
    FrontAndBack = (RawGlEnum)::gl::GL_FRONT_AND_BACK,
};

enum class FrontFaceDirection : RawGlEnum
{
    CCW = (RawGlEnum)::gl::GL_CCW,
    CW = (RawGlEnum)::gl::GL_CW,
};

enum class BlendingFactor : RawGlEnum
{
    Zero = (RawGlEnum)::gl::GL_ZERO,
    One = (RawGlEnum)::gl::GL_ONE,
    SrcColor = (RawGlEnum)::gl::GL_SRC_COLOR,
    OneMinusSrcColor = (RawGlEnum)::gl::GL_ONE_MINUS_SRC_COLOR,
    DstColor = (RawGlEnum)::gl::GL_DST_COLOR,
    OneMinusDstColor = (RawGlEnum)::gl::GL_ONE_MINUS_DST_COLOR,
    SrcAlpha = (RawGlEnum)::gl::GL_SRC_ALPHA,
    OneMinusSrcAlpha = (RawGlEnum)::gl::GL_ONE_MINUS_SRC_ALPHA,
    DstAlpha = (RawGlEnum)::gl::GL_DST_ALPHA,
    OneMinusDstAlpha = (RawGlEnum)::gl::GL_ONE_MINUS_DST_ALPHA,
    ConstantColor = (RawGlEnum)::gl::GL_CONSTANT_COLOR,
    OneMinusConstantColor = (RawGlEnum)::gl::GL_ONE_MINUS_CONSTANT_COLOR,
    ConstantAlpha = (RawGlEnum)::gl::GL_CONSTANT_ALPHA,
    OneMinusConstantAlpha = (RawGlEnum)::gl::GL_ONE_MINUS_CONSTANT_ALPHA,
    SrcAlphaSaturate = (RawGlEnum)::gl::GL_SRC_ALPHA_SATURATE,
    Src1Color = (RawGlEnum)::gl::GL_SRC1_COLOR,
    OneMinusSrc1Color = (RawGlEnum)::gl::GL_ONE_MINUS_SRC1_COLOR,
    Src1Alpha = (RawGlEnum)::gl::GL_SRC1_ALPHA,
    OneMinusSrc1Alpha = (RawGlEnum)::gl::GL_ONE_MINUS_SRC1_ALPHA,
};

enum class AlphaFunction : RawGlEnum
{
    Always = (RawGlEnum)::gl::GL_ALWAYS,
    Equal = (RawGlEnum)::gl::GL_EQUAL,
    GEqual = (RawGlEnum)::gl::GL_GEQUAL,
    Greater = (RawGlEnum)::gl::GL_GREATER,
    LEqual = (RawGlEnum)::gl::GL_LEQUAL,
    Less = (RawGlEnum)::gl::GL_LESS,
    Never = (RawGlEnum)::gl::GL_NEVER,
    NotEqual = (RawGlEnum)::gl::GL_NOTEQUAL,
};

class RenderState final
{
public:
    RenderState(const RenderState&) = default;

    RenderState(RenderState&&) = default;

    RenderState& operator=(const RenderState&) = default;

    RenderState& operator=(RenderState&&) = default;

    explicit RenderState() = default;

    ~RenderState() = default;

    void apply(bool force = false) const;

    void setBlend(bool enabled);

    void setBlendSrc(BlendingFactor blend);

    void setBlendDst(BlendingFactor blend);

    void setCullFace(bool enabled);

    void setCullFaceSide(CullFaceMode side);

    void setFrontFace(FrontFaceDirection winding);

    void setDepthTest(bool enabled);

    void setDepthWrite(bool enabled);

    void setDepthFunction(AlphaFunction func);

    void setLineWidth(::gl::GLfloat width);

    void setLineSmooth(bool enabled);

    static void initDefaults();

    static void enableDepthWrite();

    void merge(const RenderState& other);

private:
    template<typename T, const T DefaultValue>
    struct DefaultedOptional final
    {
        boost::optional<T> value{};

        T get() const
        {
            return value.get_value_or( DefaultValue );
        }

        void reset()
        {
            value.reset();
        }

        void setDefault()
        {
            value = DefaultValue;
        }

        bool isInitialized() const
        {
            return value.is_initialized();
        }

        bool operator!=(const DefaultedOptional<T, DefaultValue>& rhs) const
        {
            return value != rhs.value;
        }

        DefaultedOptional<T, DefaultValue>& operator=(T rhs)
        {
            value = rhs;
            return *this;
        }

        void merge(const DefaultedOptional<T, DefaultValue>& other)
        {
            if( other.isInitialized() )
                *this = other;
        }
    };

    struct DefaultedOptionalF final
    {
        const ::gl::GLfloat DefaultValue;

        explicit DefaultedOptionalF(::gl::GLfloat defaultValue)
            : DefaultValue{ defaultValue }
        {}

        DefaultedOptionalF& operator=(const DefaultedOptionalF& rhs)
        {
            BOOST_ASSERT( DefaultValue == rhs.DefaultValue );

            value = rhs.value;
            return *this;
        }

        boost::optional<::gl::GLfloat> value{};

        ::gl::GLfloat get() const
        {
            return value.get_value_or( DefaultValue );
        }

        void reset()
        {
            value.reset();
        }

        void setDefault()
        {
            value = DefaultValue;
        }

        bool isInitialized() const
        {
            return value.is_initialized();
        }

        bool operator!=(const DefaultedOptionalF& rhs) const
        {
            return value != rhs.value;
        }

        DefaultedOptionalF& operator=(::gl::GLfloat rhs)
        {
            value = rhs;
            return *this;
        }

        void merge(const DefaultedOptionalF& other)
        {
            if( other.isInitialized() )
                *this = other;
        }
    };

    // States
    DefaultedOptional<bool, false> m_cullFaceEnabled;

    DefaultedOptional<bool, true> m_depthTestEnabled;

    DefaultedOptional<bool, true> m_depthWriteEnabled;

    DefaultedOptional<AlphaFunction, AlphaFunction::Less> m_depthFunction;

    DefaultedOptional<bool, true> m_blendEnabled;

    DefaultedOptional<BlendingFactor, BlendingFactor::SrcAlpha> m_blendSrc;

    DefaultedOptional<BlendingFactor, BlendingFactor::OneMinusSrcAlpha> m_blendDst;

    DefaultedOptional<CullFaceMode, CullFaceMode::Back> m_cullFaceSide;

    DefaultedOptional<FrontFaceDirection, FrontFaceDirection::CW> m_frontFace;

    DefaultedOptionalF m_lineWidth{ 1.0f };

    DefaultedOptional<bool, true> m_lineSmooth;

    static inline RenderState& getCurrentState();
};
}
}
