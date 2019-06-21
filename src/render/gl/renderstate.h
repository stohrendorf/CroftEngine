#pragma once

#include "glassert.h"

#include <boost/optional.hpp>

namespace render
{
namespace gl
{
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

    void setBlendSrc(::gl::BlendingFactor blend);

    void setBlendDst(::gl::BlendingFactor blend);

    void setCullFace(bool enabled);

    void setCullFaceSide(::gl::CullFaceMode side);

    void setFrontFace(::gl::FrontFaceDirection winding);

    void setDepthTest(bool enabled);

    void setDepthWrite(bool enabled);

    void setDepthFunction(::gl::DepthFunction func);

    void setLineWidth(float width);

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
            return value.get_value_or(DefaultValue);
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
            if(other.isInitialized())
                *this = other;
        }
    };

    struct DefaultedOptionalF final
    {
        const float DefaultValue;

        explicit DefaultedOptionalF(float defaultValue)
            : DefaultValue{defaultValue}
        {
        }

        DefaultedOptionalF& operator=(const DefaultedOptionalF& rhs)
        {
            BOOST_ASSERT(DefaultValue == rhs.DefaultValue);

            value = rhs.value;
            return *this;
        }

        boost::optional<float> value{};

        float get() const
        {
            return value.get_value_or(DefaultValue);
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

        DefaultedOptionalF& operator=(float rhs)
        {
            value = rhs;
            return *this;
        }

        void merge(const DefaultedOptionalF& other)
        {
            if(other.isInitialized())
                *this = other;
        }
    };

    // States
    DefaultedOptional<bool, false> m_cullFaceEnabled;

    DefaultedOptional<bool, true> m_depthTestEnabled;

    DefaultedOptional<bool, true> m_depthWriteEnabled;

    DefaultedOptional<::gl::DepthFunction, ::gl::DepthFunction::Less> m_depthFunction;

    DefaultedOptional<bool, true> m_blendEnabled;

    DefaultedOptional<::gl::BlendingFactor, ::gl::BlendingFactor::SrcAlpha> m_blendSrc;

    DefaultedOptional<::gl::BlendingFactor, ::gl::BlendingFactor::OneMinusSrcAlpha> m_blendDst;

    DefaultedOptional<::gl::CullFaceMode, ::gl::CullFaceMode::Back> m_cullFaceSide;

    DefaultedOptional<::gl::FrontFaceDirection, ::gl::FrontFaceDirection::Cw> m_frontFace;

    DefaultedOptionalF m_lineWidth{1.0f};

    DefaultedOptional<bool, true> m_lineSmooth;

    static inline RenderState& getCurrentState();
};
} // namespace gl
} // namespace render
