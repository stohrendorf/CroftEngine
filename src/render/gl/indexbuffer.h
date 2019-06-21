#pragma once

#include "bindableresource.h"
#include "gsl-lite.hpp"
#include "typetraits.h"

#include <vector>

namespace render
{
namespace gl
{
class IndexBuffer : public BindableResource
{
public:
    explicit IndexBuffer(const std::string& label = {})
        : BindableResource{
            ::gl::genBuffers,
            [](const uint32_t handle) { ::gl::bindBuffer(::gl::BufferTargetARB::ElementArrayBuffer, handle); },
            ::gl::deleteBuffers,
            ::gl::ObjectIdentifier::Buffer,
            label}
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    const void* map()
    {
        bind();
        const void* data
            = GL_ASSERT_FN(::gl::mapBuffer(::gl::BufferTargetARB::ElementArrayBuffer, ::gl::BufferAccessARB::ReadOnly));
        return data;
    }

    static void unmap()
    {
        GL_ASSERT(::gl::unmapBuffer(::gl::BufferTargetARB::ElementArrayBuffer));
    }

    template<typename T>
    void setData(const gsl::not_null<const T*>& indexData, const ::gl::core::SizeType indexCount, const bool dynamic)
    {
        Expects(indexCount >= 0);

        bind();

        GL_ASSERT(::gl::bufferData(::gl::BufferTargetARB::ElementArrayBuffer,
                                   gsl::narrow<std::size_t>(sizeof(T) * indexCount),
                                   indexData.get(),
                                   dynamic ? ::gl::BufferUsageARB::DynamicDraw : ::gl::BufferUsageARB::StaticDraw));

        m_indexCount = indexCount;
        m_storageType = TypeTraits<T>::DrawElementsType;
    }

    template<typename T>
    void setData(const std::vector<T>& data, bool dynamic)
    {
        setData(gsl::not_null<const T*>(data.data()), gsl::narrow<::gl::core::SizeType>(data.size()), dynamic);
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    template<typename T>
    void setSubData(const gsl::not_null<const T*>& indexData,
                    ::gl::core::SizeType indexStart,
                    ::gl::core::SizeType indexCount)
    {
        Expects(indexStart >= 0);
        Expects(indexCount >= 0);
        if(indexStart + indexCount > m_indexCount)
        {
            BOOST_THROW_EXCEPTION(std::out_of_range{"Sub-range exceeds buffer range"});
        }

        if(!m_storageType.is_initialized())
        {
            BOOST_THROW_EXCEPTION(std::logic_error{"Buffer is not initialized"});
        }

        if(TypeTraits<T>::DrawElementsType != *m_storageType)
        {
            BOOST_THROW_EXCEPTION(std::logic_error{"Incompatible storage type for buffer sub-data"});
        }

        bind();

        GL_ASSERT(::gl::bufferSubData(::gl::BufferTargetARB::ElementArrayBuffer,
                                      gsl::narrow<std::intptr_t>(indexStart * sizeof(T)),
                                      gsl::narrow<std::size_t>(indexCount * sizeof(T)),
                                      indexData.get()));
    }

    void draw(const ::gl::PrimitiveType mode) const
    {
        Expects(m_storageType.is_initialized());
        GL_ASSERT(::gl::drawElements(mode, m_indexCount, *m_storageType, nullptr));
    }

    ::gl::core::SizeType getIndexCount() const
    {
        return m_indexCount;
    }

private:
    ::gl::core::SizeType m_indexCount = 0;

    boost::optional<::gl::DrawElementsType> m_storageType;
};
} // namespace gl
} // namespace render
