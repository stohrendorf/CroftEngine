#pragma once

#include "bufferparameter.h"
#include "uniformparameter.h"

#include <boost/container/flat_map.hpp>
#include <functional>
#include <string>

namespace render::material
{
template<typename T>
class SingleMaterialParameterOverrider final
{
public:
  [[nodiscard]] const std::function<T>* find(const std::string& name) const
  {
    const auto it = m_setters.find(name);
    if(it != m_setters.end())
      return &it->second;

    return nullptr;
  }

  void bind(const std::string& name, const std::function<T>& setter)
  {
    m_setters[name] = setter;
  }

  void bind(const std::string& name, std::function<T>&& setter)
  {
    m_setters[name] = std::move(setter);
  }

private:
  boost::container::flat_map<std::string, std::function<T>> m_setters;
};

class MaterialParameterOverrider
{
public:
  virtual ~MaterialParameterOverrider() = default;

  [[nodiscard]] const std::function<UniformParameter::UniformValueSetter>*
    findUniformSetter(const std::string& name) const
  {
    return m_uniformSetters.find(name);
  }

  [[nodiscard]] const std::function<UniformBlockParameter::BufferBinder>*
    findUniformBlockBinder(const std::string& name) const
  {
    return m_uniformBlockBinders.find(name);
  }

  [[nodiscard]] const std::function<BufferParameter::BufferBinder>*
    findShaderStorageBlockBinder(const std::string& name) const
  {
    return m_bufferBinders.find(name);
  }

  void bind(const std::string& name, const std::function<UniformParameter::UniformValueSetter>& setter)
  {
    m_uniformSetters.bind(name, setter);
  }

  void bind(const std::string& name, std::function<UniformParameter::UniformValueSetter>&& setter)
  {
    m_uniformSetters.bind(name, std::move(setter));
  }

  void bind(const std::string& name, const std::function<BufferParameter::BufferBinder>& binder)
  {
    m_bufferBinders.bind(name, binder);
  }

  void bind(const std::string& name, std::function<BufferParameter::BufferBinder>&& binder)
  {
    m_bufferBinders.bind(name, std::move(binder));
  }

  void bind(const std::string& name, const std::function<UniformBlockParameter::BufferBinder>& binder)
  {
    m_uniformBlockBinders.bind(name, binder);
  }

  void bind(const std::string& name, std::function<UniformBlockParameter::BufferBinder>&& binder)
  {
    m_uniformBlockBinders.bind(name, std::move(binder));
  }

private:
  SingleMaterialParameterOverrider<UniformParameter::UniformValueSetter> m_uniformSetters;
  SingleMaterialParameterOverrider<UniformBlockParameter::BufferBinder> m_uniformBlockBinders;
  SingleMaterialParameterOverrider<BufferParameter::BufferBinder> m_bufferBinders;
};
} // namespace render::material
