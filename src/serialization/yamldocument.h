#pragma once

#include "serialization_fwd.h"

#include <filesystem>
#include <fstream>
#include <gsl/gsl-lite.hpp>
#include <ryml.hpp>
#include <type_traits>

namespace serialization
{
template<bool Loading>
class YAMLDocument
{
private:
  const std::filesystem::path m_filename;
  std::string m_buffer;
  ryml::Tree m_tree;

public:
  explicit YAMLDocument(const std::filesystem::path& filename)
      : m_filename{filename}
  {
    if constexpr(Loading)
    {
      std::ifstream file{filename, std::ios::in};
      Expects(file.is_open());
      file.seekg(0, std::ios::end);
      const auto size = static_cast<std::size_t>(file.tellg());
      file.seekg(0, std::ios::beg);

      m_buffer.resize(size);
      file.read(&m_buffer[0], size);
      m_tree = ryml::parse(c4::to_csubstr(filename.string()), c4::to_csubstr(m_buffer));
    }
    else
    {
      std::ofstream file{filename, std::ios::out | std::ios::trunc};
      Expects(file.is_open());
      m_tree.rootref() |= ryml::MAP;
    }
  }

  template<typename T, typename TContext, bool DelayLoading = Loading>
  auto load(const std::string& key, TContext& context) -> std::enable_if_t<DelayLoading, T>
  {
    const std::string oldLocale = setlocale(LC_NUMERIC, nullptr);
    setlocale(LC_NUMERIC, "C");

    Serializer<TContext> ser{m_tree.rootref()[c4::to_csubstr(key)], context, true, nullptr};
    auto result = access<T>::callCreate(ser);
    ser.processQueues();

    setlocale(LC_NUMERIC, oldLocale.c_str());
    return result;
  }

  template<typename T, typename TContext, bool DelayLoading = Loading>
  auto load(const std::string& key, TContext& context, T& data) -> std::enable_if_t<DelayLoading, void>
  {
    const std::string oldLocale = setlocale(LC_NUMERIC, nullptr);
    setlocale(LC_NUMERIC, "C");

    Serializer<TContext> ser{m_tree.rootref()[c4::to_csubstr(key)], context, true, nullptr};
    access<T>::callSerializeOrLoad(data, ser);
    ser.processQueues();

    setlocale(LC_NUMERIC, oldLocale.c_str());
  }

  template<typename T, typename TContext, bool DelayLoading = Loading>
  auto save(const std::string& key, TContext& context, T& data) -> std::enable_if_t<!DelayLoading, void>
  {
    const std::string oldLocale = setlocale(LC_NUMERIC, nullptr);
    setlocale(LC_NUMERIC, "C");

    Serializer ser{m_tree.rootref()[m_tree.copy_to_arena(c4::to_csubstr(key))], context, false, nullptr};
    access<T>::callSerializeOrSave(data, ser);
    ser.processQueues();

    setlocale(LC_NUMERIC, oldLocale.c_str());
  }

  template<bool DelayLoading = Loading>
  auto write() const -> std::enable_if_t<!DelayLoading, void>
  {
    std::ofstream file{m_filename, std::ios::out | std::ios::trunc};
    Expects(file.is_open());
    file << m_tree.rootref();
  }

  template<bool DelayLoading = Loading>
  auto operator[](const std::string& key) -> std::enable_if_t<DelayLoading, ryml::NodeRef>
  {
    return m_tree.rootref()[c4::to_csubstr(key)];
  }
};
} // namespace serialization
