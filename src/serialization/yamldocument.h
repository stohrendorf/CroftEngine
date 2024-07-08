#pragma once

#include "access.h" // IWYU pragma: keep
#include "serialization.h"

#include <clocale>
#include <filesystem>
#include <fstream>
#include <gsl/gsl-lite.hpp>
#include <ryml.hpp>
#include <string>
#include <type_traits>

namespace serialization
{
template<bool Loading>
class YAMLDocument
{
private:
  std::filesystem::path m_filename;
  std::string m_buffer;
  ryml::Tree m_tree;

  struct CustomErrorCallbacks
  {
  public:
    explicit CustomErrorCallbacks()
        : m_callbacks{ryml::get_callbacks()}
    {
      ryml::set_callbacks(
        ryml::Callbacks{nullptr,
                        [](size_t length, void* /*hint*/, void* /*user_data*/) -> gsl::owner<void*>
                        {
                          return new char[length];
                        },
                        [](gsl::owner<void*> mem, size_t /*length*/, void* /*user_data*/)
                        {
                          delete[] static_cast<char*>(mem);
                        },
                        [](const char* msg, size_t msg_len, ryml::Location /*location*/, void* /*user_data*/)
                        {
                          const std::string msgStr{msg, msg_len};
                          SERIALIZER_EXCEPTION(msgStr);
                        }});
    }

    ~CustomErrorCallbacks()
    {
      ryml::set_callbacks(m_callbacks);
    }

  private:
    ryml::Callbacks m_callbacks;
  };

public:
  explicit YAMLDocument(const std::filesystem::path& filename)
      : m_filename{filename}
  {
    if constexpr(Loading)
    {
      std::ifstream file{filename, std::ios::in | std::ios::binary};
      gsl_Assert(file.is_open());
      file.seekg(0, std::ios::end);
      const auto size = static_cast<std::streamsize>(file.tellg());
      file.seekg(0, std::ios::beg);

      m_buffer.resize(size);
      file.read(m_buffer.data(), size);
      m_tree = ryml::parse_in_arena(c4::to_csubstr(m_buffer));
    }
    else
    {
      std::ofstream file{filename, std::ios::out | std::ios::trunc};
      gsl_Assert(file.is_open());
      m_tree.rootref() |= ryml::MAP;
    }
  }

  template<bool Lazy = Loading>
  explicit YAMLDocument(std::string data, std::enable_if_t<Lazy, bool> = true)
      : m_buffer{std::move(data)}
  {
    static_assert(Lazy);

    m_tree = ryml::parse_in_arena(c4::to_csubstr(m_buffer));
  }

  template<typename T, typename TContext, bool DelayLoading = Loading>
  auto deserialize(const std::string& key, TContext& context) -> std::enable_if_t<DelayLoading, T>
  {
    const std::string oldLocale = gsl::not_null{setlocale(LC_NUMERIC, nullptr)}.get();
    setlocale(LC_NUMERIC, "C");

    CustomErrorCallbacks callbacks{};

    Deserializer<TContext> ser{m_tree.rootref()[c4::to_csubstr(key)], context, true, nullptr};
    auto result = access<T, true>::dispatch(ser);
    ser.processQueues();

    setlocale(LC_NUMERIC, oldLocale.c_str());
    return result;
  }

  template<typename T, typename TContext, bool DelayLoading = Loading>
  auto deserialize(const std::string& key,
                   const gsl::not_null<TContext*>& context,
                   T& data) -> std::enable_if_t<DelayLoading, void>
  {
    const std::string oldLocale = gsl::not_null{setlocale(LC_NUMERIC, nullptr)}.get();
    setlocale(LC_NUMERIC, "C");

    CustomErrorCallbacks callbacks{};

    Deserializer<TContext> ser{m_tree.rootref()[c4::to_csubstr(key)], context, nullptr};
    access<T, true>::dispatch(data, ser);
    ser.processQueues();

    setlocale(LC_NUMERIC, oldLocale.c_str());
  }

  template<typename T, typename TContext, bool DelayLoading = Loading>
  auto serialize(const std::string& key,
                 const gsl::not_null<TContext*>& context,
                 T& data) -> std::enable_if_t<!DelayLoading, void>
  {
    const std::string oldLocale = gsl::not_null{setlocale(LC_NUMERIC, nullptr)}.get();
    setlocale(LC_NUMERIC, "C");

    CustomErrorCallbacks callbacks{};

    Serializer<TContext> ser{m_tree.rootref()[m_tree.copy_to_arena(c4::to_csubstr(key))], context, nullptr};
    access<T, false>::dispatch(data, ser);
    ser.processQueues();

    setlocale(LC_NUMERIC, oldLocale.c_str());
  }

  template<bool DelayLoading = Loading>
  auto write() const -> std::enable_if_t<!DelayLoading, void>
  {
    std::ofstream file{m_filename, std::ios::out | std::ios::trunc};
    gsl_Assert(file.is_open());
    file << m_tree.rootref();
  }

  template<bool DelayLoading = Loading>
  auto operator[](const std::string& key) -> std::enable_if_t<DelayLoading, ryml::NodeRef>
  {
    return m_tree.rootref()[c4::to_csubstr(key)];
  }

  template<bool DelayLoading = Loading>
  auto getRoot() -> std::enable_if_t<DelayLoading, ryml::NodeRef>
  {
    return m_tree.rootref();
  }
};
} // namespace serialization
