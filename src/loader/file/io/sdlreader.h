#pragma once

#include "type_safe/integer.hpp"

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4702)
#endif
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#ifdef _MSC_VER
#  pragma warning(pop)
#endif
#include <boost/throw_exception.hpp>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
#include <zlib.h>

namespace qs
{
template<typename, typename>
struct quantity;
}

namespace loader::file::io
{
using DataStreamBuf = boost::iostreams::filtering_istreambuf;

class SDLReader
{
public:
  SDLReader(const SDLReader&) = delete;

  SDLReader& operator=(const SDLReader&) = delete;

  SDLReader& operator=(SDLReader&&) = delete;

  explicit SDLReader(std::shared_ptr<DataStreamBuf> stream)
      : m_streamBuf{std::move(stream)}
      , m_stream{m_streamBuf.get()}
  {
  }

  SDLReader(SDLReader&& rhs) noexcept
      : m_memory{std::move(rhs.m_memory)}
      , m_file{std::move(rhs.m_file)}
      , m_array{std::move(rhs.m_array)}
      , m_streamBuf{std::move(rhs.m_streamBuf)}
      , m_stream{m_streamBuf.get()}
  {
  }

  explicit SDLReader(const std::filesystem::path& filename)
      : m_file{std::make_unique<boost::iostreams::file>(
        filename.string(), std::ios::in | std::ios::binary, std::ios::in | std::ios::binary)}
      , m_streamBuf{std::make_shared<DataStreamBuf>(*m_file)}
      , m_stream{m_streamBuf.get()}
  {
  }

  explicit SDLReader(std::vector<char> data)
      : m_memory{std::move(data)}
      , m_array{std::make_unique<boost::iostreams::array>(m_memory.data(), m_memory.size())}
      , m_streamBuf{std::make_shared<DataStreamBuf>(*m_array)}
      , m_stream{m_streamBuf.get()}
  {
  }

  ~SDLReader() = default;

  static SDLReader decompress(const std::vector<uint8_t>& compressed, const size_t uncompressedSize)
  {
    std::vector<char> uncomp_buffer(uncompressedSize);

    auto actuallyUncompressedSize = static_cast<uLongf>(uncompressedSize);
    if(uncompress(reinterpret_cast<Bytef*>(uncomp_buffer.data()), // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                  &actuallyUncompressedSize,
                  compressed.data(),
                  static_cast<uLong>(compressed.size()))
       != Z_OK)
      BOOST_THROW_EXCEPTION(std::runtime_error("Decompression failed"));

    if(actuallyUncompressedSize != uncompressedSize)
      BOOST_THROW_EXCEPTION(std::runtime_error("Decompressed size mismatch"));

    SDLReader reader(std::move(uncomp_buffer));
    if(!reader.isOpen())
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create reader from decompressed memory"));

    return reader;
  }

  [[nodiscard]] bool isOpen() const
  {
    return !m_stream.bad();
  }

  [[nodiscard]] std::streampos tell() const
  {
    return m_stream.tellg();
  }

  [[nodiscard]] std::streamsize size() const
  {
    const auto pos = m_stream.tellg();
    m_stream.seekg(0, std::ios::end);
    const auto size = m_stream.tellg();
    m_stream.seekg(pos, std::ios::beg);
    return size;
  }

  void skip(const std::streamoff delta)
  {
    m_stream.seekg(delta, std::ios::cur);
  }

  void seek(const std::streampos& position)
  {
    m_stream.seekg(position, std::ios::beg);
  }

  template<typename T>
  void readBytes(T* dest, const size_t n)
  {
    static_assert(std::is_integral_v<T> && sizeof(T) == 1, "readBytes() only allowed for byte-compatible data");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    m_stream.read(reinterpret_cast<char*>(dest), gsl::narrow<std::streamsize>(n));
    if(static_cast<size_t>(m_stream.gcount()) != n)
    {
      BOOST_THROW_EXCEPTION(std::runtime_error("EOF unexpectedly reached"));
    }
  }

  template<typename T, typename... Args>
  using PtrProducer = std::unique_ptr<T>(SDLReader&, Args... args);

  template<typename T, typename... Args>
  using StackProducer = T(SDLReader&, Args... args);

  template<typename T, typename... Args>
  void readVector(std::vector<T>& elements, size_t count, PtrProducer<T, Args...> producer, Args... args)
  {
    elements.clear();
    appendVector(elements, count, producer, args...);
  }

  template<typename T, typename... Args>
  void readVector(std::vector<T>& elements, size_t count, StackProducer<T, Args...> producer, Args... args)
  {
    elements.clear();
    appendVector(elements, count, producer, args...);
  }

  template<typename T, typename... Args>
  void appendVector(std::vector<T>& elements, size_t count, PtrProducer<T, Args...> producer, Args... args)
  {
    elements.reserve(elements.size() + count);
    for(size_t i = 0; i < count; ++i)
    {
      elements.emplace_back(*producer(*this, args...));
    }
  }

  template<typename T, typename... Args>
  void appendVector(std::vector<T>& elements, size_t count, StackProducer<T, Args...> producer, Args... args)
  {
    elements.reserve(elements.size() + count);
    for(size_t i = 0; i < count; ++i)
    {
      elements.emplace_back(producer(*this, args...));
    }
  }

  template<typename T>
  void readVector(std::vector<T>& elements, size_t count)
  {
    elements.clear();
    elements.reserve(count);
    for(size_t i = 0; i < count; ++i)
    {
      elements.emplace_back(read<T>());
    }
  }

  void readVector(std::vector<uint8_t>& elements, const size_t count)
  {
    elements.clear();
    elements.resize(count);
    readBytes(elements.data(), count);
  }

  void readVector(std::vector<int8_t>& elements, const size_t count)
  {
    elements.clear();
    elements.resize(count);
    readBytes(elements.data(), count);
  }

  template<typename T>
  [[nodiscard]] T read()
  {
    return ReadTraits<T>::read(m_stream);
  }

  [[nodiscard]] uint8_t readU8()
  {
    return read<uint8_t>();
  }

  [[nodiscard]] int8_t readI8()
  {
    return read<int8_t>();
  }

  [[nodiscard]] uint16_t readU16()
  {
    return read<uint16_t>();
  }

  [[nodiscard]] int16_t readI16()
  {
    return read<int16_t>();
  }

  [[nodiscard]] uint32_t readU32()
  {
    return read<uint32_t>();
  }

  [[nodiscard]] int32_t readI32()
  {
    return read<int32_t>();
  }

  [[nodiscard]] float readF()
  {
    return read<float>();
  }

private:
  // Do not change the order of these member variables.
  std::vector<char> m_memory;

  std::unique_ptr<boost::iostreams::file> m_file;

  std::unique_ptr<boost::iostreams::array> m_array;

  std::shared_ptr<DataStreamBuf> m_streamBuf;

  mutable std::istream m_stream;

  template<typename T, int dataSize, bool isIntegral>
  struct SwapTraits
  {
  };

  template<typename T, int dataSize>
  struct SwapTraits<T, dataSize, true>
  {
    static void doSwap(T& /*data*/)
    {
      // TODO For now, no endian conversion
    }
  };

  template<typename T, int dataSize>
  struct SwapTraits<type_safe::integer<T>, dataSize, false>
  {
    static void doSwap(type_safe::integer<T>& data)
    {
      auto tmp = data.get();
      SwapTraits<T, sizeof(T), (std::is_integral_v<T> || std::is_floating_point_v<T>)>::doSwap(tmp);
      data = type_safe::integer<T>(tmp);
    }
  };

  template<typename T, int dataSize, typename U>
  struct SwapTraits<qs::quantity<U, T>, dataSize, false>
  {
    static void doSwap(qs::quantity<U, T>& data)
    {
      auto tmp = data.get();
      SwapTraits<T, sizeof(T), (std::is_integral_v<T> || std::is_floating_point_v<T>)>::doSwap(tmp);
      data = qs::quantity<U, T>(tmp);
    }
  };

  template<typename T>
  struct ReadTraits
  {
    [[nodiscard]] static T read(std::istream& stream)
    {
      T result;
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      stream.read(reinterpret_cast<char*>(&result), sizeof(T));
      if(stream.gcount() != sizeof(T))
      {
        BOOST_THROW_EXCEPTION(std::runtime_error("EOF unexpectedly reached"));
      }

      SwapTraits<T, sizeof(T), std::is_integral_v<T> || std::is_floating_point_v<T>>::doSwap(result);

      return result;
    }
  };

  template<typename T>
  struct ReadTraits<type_safe::integer<T>>
  {
    [[nodiscard]] static type_safe::integer<T> read(std::istream& stream)
    {
      return type_safe::integer<T>{ReadTraits<T>::read(stream)};
    }
  };
};
} // namespace loader::file::io
