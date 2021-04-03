#pragma once

#include "pixel.h"

#include <boost/throw_exception.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <gsl/gsl-lite.hpp>
#include <vector>

namespace gl
{
namespace detail
{
template<typename T, size_t N>
struct FastFill
{
  static_assert(sizeof(T) == N, "Type size mismatch");

  static void fill(const gsl::not_null<T*>& data, size_t n, const T& value)
  {
    std::fill_n(data.get(), n, value);
  }
};

template<typename T>
struct FastFill<T, 1>
{
  static_assert(sizeof(T) == 1, "Type size mismatch");

  static void fill(const gsl::not_null<T*>& data, size_t n, const T& value)
  {
    std::memset(data.get(), value, n);
  }
};

template<typename T>
struct FastFill<T, 2>
{
  static_assert(sizeof(T) == 2, "Type size mismatch");

  static void fill(const gsl::not_null<T*>& data, size_t n, const T& value)
  {
    std::wmemset(data.get(), value, n);
  }
};

template<>
struct FastFill<SRGBA8, 4>
{
  static_assert(sizeof(SRGBA8) == 4, "Type size mismatch");

  static void fill(const gsl::not_null<SRGBA8*>& data, const size_t n, const SRGBA8& value)
  {
    const auto scalar = value.channels[0];
    if(scalar == value.channels[1] && scalar == value.channels[2] && scalar == value.channels[3])
      std::memset(value_ptr(data->channels), scalar, n * 4u);
    else
      std::fill_n(data.get(), n, value);
  }
};

template<typename T>
void fill(const gsl::not_null<T*>& data, size_t n, const T& value)
{
  FastFill<T, sizeof(T)>::fill(data, n, value);
}
} // namespace detail

template<typename TStorage>
class Image
{
public:
  using StorageType = TStorage;

  explicit Image(const glm::ivec2& size, const StorageType* data = nullptr)
      : m_data{}
      , m_size{size}
  {
    Expects(size.x >= 0 && size.y >= 0);

    const auto dataSize = static_cast<size_t>(size.x) * static_cast<size_t>(size.y);
    if(data == nullptr)
      m_data.resize(dataSize);
    else
      m_data.assign(data, data + dataSize);
  }

  Image()
      : Image{{0, 0}}
  {
  }

  Image(const Image&) = delete;

  Image(Image&& rhs) noexcept
      : m_data{std::move(rhs.m_data)}
      , m_size{std::exchange(rhs.m_size, {0, 0})}
  {
  }

  Image& operator=(const Image&) = delete;

  Image& operator=(Image&& rhs) noexcept
  {
    m_data = std::move(rhs.m_data);
    m_size = std::exchange(rhs.m_size, {0, 0});
    return *this;
  }

  ~Image() = default;

  [[nodiscard]] const std::vector<StorageType>& getData() const
  {
    return m_data;
  }

  StorageType* getRawData()
  {
    return m_data.data();
  }

  auto begin()
  {
    return m_data.begin();
  }

  void assign(const std::vector<StorageType>& data)
  {
    Expects(m_data.size() == data.size());
    m_data = data;
  }

  void assign(const StorageType* data, size_t size)
  {
    Expects(m_data.size() == size);
    m_data.assign(data + 0, data + size);
  }

  void assign(std::vector<StorageType>&& data)
  {
    Expects(m_data.size() == data.size());
    m_data = std::move(data);
  }

  [[nodiscard]] const auto& getSize() const
  {
    return m_size;
  }

  [[nodiscard]] StorageType& at(const glm::ivec2& xy)
  {
    if(xy.x < 0 || xy.x >= m_size.x || xy.y < 0 || xy.y >= m_size.y)
    {
      BOOST_THROW_EXCEPTION(std::out_of_range{"Image coordinates out of range"});
    }

    return m_data[xy.y * m_size.x + xy.x];
  }

  void set(const glm::ivec2& xy, const StorageType& pixel, const bool blend = false)
  {
    if(xy.x < 0 || xy.x >= m_size.x || xy.y < 0 || xy.y >= m_size.y)
      return;

    const auto o = gsl::narrow_cast<size_t>(xy.y * m_size.x + xy.x);

    if(!blend)
    {
      m_data[o] = pixel;
    }
    else
    {
      m_data[o] = mixAlpha(m_data[o], pixel);
    }
  }

  [[nodiscard]] const StorageType& at(const glm::ivec2& xy) const
  {
    if(xy.x < 0 || xy.x >= m_size.x || xy.y < 0 || xy.y >= m_size.y)
    {
      BOOST_THROW_EXCEPTION(std::out_of_range{"Image coordinates out of range"});
    }

    return m_data[xy.y * m_size.x + xy.x];
  }

  void fill(const StorageType& color)
  {
    if(!m_data.empty())
      detail::fill(gsl::not_null{m_data.data()}, m_data.size(), color);
  }

private:
  std::vector<StorageType> m_data;
  glm::ivec2 m_size{0};
};
} // namespace gl
