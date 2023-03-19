#pragma once

#include "api/gl.hpp" // IWYU pragma: export
#include "glassert.h"

#include <boost/assert.hpp>
#include <functional>
#include <string_view>

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ObjectIdentifier _Identifier>
class Resource
{
public:
  static constexpr auto Identifier = _Identifier;

  Resource(const Resource&) = delete;
  Resource(Resource&&) = delete;
  void operator=(const Resource&) = delete;
  void operator=(Resource&&) = delete;

  [[nodiscard]] auto getHandle() const
  {
    BOOST_ASSERT(m_handle != 0);
    return m_handle;
  }

protected:
  using Allocator = std::function<void(api::core::SizeType, api::core::Handle*)>;
  using Deleter = std::function<void(api::core::SizeType, api::core::Handle*)>;

  explicit Resource(Allocator allocator, Deleter deleter, const std::string_view& label)
      : m_allocator{std::move(allocator)}
      , m_deleter{std::move(deleter)}
  {
    BOOST_ASSERT(static_cast<bool>(m_allocator));
    BOOST_ASSERT(static_cast<bool>(m_deleter));

    GL_ASSERT(m_allocator(1, &m_handle));

    BOOST_ASSERT(m_handle != 0);

    setLabel(label);
  }

  virtual ~Resource()
  {
    if(m_handle == 0)
      return;

    GL_ASSERT(m_deleter(1, &m_handle));
  }

  // NOLINTNEXTLINE(readability-make-member-function-const)
  void setLabel(const std::string_view& label)
  {
    if(label.empty())
      return;

    int32_t maxLabelLength = 0;
    GL_ASSERT(api::getInteger(api::GetPName::MaxLabelLength, &maxLabelLength));
    BOOST_ASSERT(maxLabelLength > 0);

    GL_ASSERT(api::objectLabel(
      Identifier, m_handle, std::min(maxLabelLength, gsl::narrow<int32_t>(label.size())), label.data()));
  }

private:
  api::core::Handle m_handle = 0;
  Allocator m_allocator;
  Deleter m_deleter;
};
} // namespace gl
