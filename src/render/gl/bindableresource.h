#pragma once

#include "glassert.h"

#include <functional>

namespace render
{
namespace gl
{
class BindableResource
{
public:
    BindableResource(const BindableResource&) = delete;

    BindableResource& operator=(const BindableResource&) = delete;

    void bind() const
    {
        if(m_handle == 0)
            return;

        GL_ASSERT(m_binder(m_handle));
    }

    void unbind() const
    {
        GL_ASSERT(m_binder(0));
    }

    auto getHandle() const
    {
        BOOST_ASSERT(m_handle != 0);
        return m_handle;
    }

protected:
    using Allocator = std::function<void(::gl::core::SizeType, uint32_t*)>;
    using Binder = std::function<void(uint32_t)>;
    using Deleter = std::function<void(::gl::core::SizeType, uint32_t*)>;

    explicit BindableResource(const Allocator& allocator,
                              const Binder& binder,
                              const Deleter& deleter,
                              const ::gl::ObjectIdentifier identifier,
                              const std::string& label)
        : m_allocator{allocator}
        , m_binder{binder}
        , m_deleter{deleter}
    {
        BOOST_ASSERT(static_cast<bool>(allocator));
        BOOST_ASSERT(static_cast<bool>(binder));
        BOOST_ASSERT(static_cast<bool>(deleter));

        GL_ASSERT(m_allocator(1, &m_handle));

        BOOST_ASSERT(m_handle != 0);

        if(!label.empty())
        {
            // An object must be created (not only reserved) to be able to have a label assigned;
            // for certain types of resources, this may fail, e.g. programs which must be linked
            // before they are considered "created".
            bind();
            setLabel(identifier, label);
            unbind();
        }
    }

    explicit BindableResource(BindableResource&& rhs) noexcept
        : m_handle{rhs.m_handle}
        , m_allocator{move(rhs.m_allocator)}
        , m_binder{move(rhs.m_binder)}
        , m_deleter{move(rhs.m_deleter)}
    {
        rhs.m_handle = 0;
    }

    BindableResource& operator=(BindableResource&& rhs)
    {
        m_handle = rhs.m_handle;
        m_allocator = move(rhs.m_allocator);
        m_binder = move(rhs.m_binder);
        m_deleter = move(rhs.m_deleter);
        rhs.m_handle = 0;
        return *this;
    }

    virtual ~BindableResource()
    {
        if(m_handle == 0)
            return;

        unbind();
        GL_ASSERT(m_deleter(1, &m_handle));
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void setLabel(const ::gl::ObjectIdentifier identifier, const std::string& label)
    {
        int32_t maxLabelLength = 0;
        GL_ASSERT(::gl::getIntegerv(::gl::GetPName::MaxLabelLength, &maxLabelLength));
        BOOST_ASSERT(maxLabelLength > 0);

        GL_ASSERT(::gl::objectLabel(identifier,
                                    m_handle,
                                    -1,
                                    label.empty() ? nullptr
                                                  : label.substr(0, static_cast<std::size_t>(maxLabelLength)).c_str()));
    }

private:
    uint32_t m_handle = 0;

    Allocator m_allocator;

    Binder m_binder;

    Deleter m_deleter;
};
} // namespace gl
} // namespace render
