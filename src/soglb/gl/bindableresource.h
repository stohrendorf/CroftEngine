#pragma once

#include "api/gl.hpp"
#include "glassert.h"

#include <boost/assert.hpp>
#include <functional>

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

  [[nodiscard]] auto getHandle() const
  {
    BOOST_ASSERT(m_handle != 0);
    return m_handle;
  }

protected:
  using Allocator = std::function<void(api::core::SizeType, api::core::Handle*)>;
  using Binder = std::function<void(uint32_t)>;
  using Deleter = std::function<void(api::core::SizeType, api::core::Handle*)>;

  explicit BindableResource(Allocator allocator,
                            Binder binder,
                            Deleter deleter,
                            const api::ObjectIdentifier identifier,
                            const std::string& label)
      : m_allocator{std::move(allocator)}
      , m_binder{std::move(binder)}
      , m_deleter{std::move(deleter)}
  {
    BOOST_ASSERT(static_cast<bool>(m_allocator));
    BOOST_ASSERT(static_cast<bool>(m_binder));
    BOOST_ASSERT(static_cast<bool>(m_deleter));

    GL_ASSERT(m_allocator(1, &m_handle));

    BOOST_ASSERT(m_handle != 0);

    if(!label.empty())
    {
      setLabel(identifier, label);
    }
  }

  BindableResource(BindableResource&& rhs) noexcept
      : m_handle{std::exchange(rhs.m_handle, 0)}
      , m_allocator{move(rhs.m_allocator)}
      , m_binder{move(rhs.m_binder)}
      , m_deleter{move(rhs.m_deleter)}
  {
  }

  BindableResource& operator=(BindableResource&& rhs) noexcept
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

  // NOLINTNEXTLINE(readability-make-member-function-const)
  void setLabel(const api::ObjectIdentifier identifier, const std::string& label)
  {
    int32_t maxLabelLength = 0;
    GL_ASSERT(api::getIntegerv(api::GetPName::MaxLabelLength, &maxLabelLength));
    BOOST_ASSERT(maxLabelLength > 0);

    GL_ASSERT(
      api::objectLabel(identifier,
                       m_handle,
                       -1,
                       label.empty() ? nullptr : label.substr(0, static_cast<std::size_t>(maxLabelLength)).c_str()));
  }

private:
  api::core::Handle m_handle = 0;
  Allocator m_allocator;
  Binder m_binder;
  Deleter m_deleter;
};
} // namespace gl
