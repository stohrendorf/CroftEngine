#pragma once

#include "bufferparameter.h"
#include "uniformparameter.h"

#include <boost/container/flat_map.hpp>

namespace render::scene
{
class MaterialParameterOverrider
{
public:
  virtual ~MaterialParameterOverrider() = default;

  const std::function<UniformParameter::UniformValueSetter>* findUniformSetter(const std::string& name) const
  {
    const auto it = m_uniformSetters.find(name);
    if(it != m_uniformSetters.end())
      return &it->second;

    return nullptr;
  }

  const std::function<UniformBlockParameter::BufferBinder>* findUniformBlockBinder(const std::string& name) const
  {
    const auto it = m_uniformBlockBinders.find(name);
    if(it != m_uniformBlockBinders.end())
      return &it->second;

    return nullptr;
  }

  const std::function<BufferParameter::BufferBinder>* findShaderStorageBlockBinder(const std::string& name) const
  {
    const auto it = m_bufferBinders.find(name);
    if(it != m_bufferBinders.end())
      return &it->second;

    return nullptr;
  }

  void bind(const std::string& name, const std::function<UniformParameter::UniformValueSetter>& setter)
  {
    m_uniformSetters[name] = setter;
  }

  void bind(const std::string& name, std::function<UniformParameter::UniformValueSetter>&& setter)
  {
    m_uniformSetters[name] = std::move(setter);
  }

  void bind(const std::string& name, const std::function<BufferParameter::BufferBinder>& binder)
  {
    m_bufferBinders[name] = binder;
  }

  void bind(const std::string& name, std::function<BufferParameter::BufferBinder>&& binder)
  {
    m_bufferBinders[name] = std::move(binder);
  }

  void bind(const std::string& name, const std::function<UniformBlockParameter::BufferBinder>& binder)
  {
    m_uniformBlockBinders[name] = binder;
  }

  void bind(const std::string& name, std::function<UniformBlockParameter::BufferBinder>&& binder)
  {
    m_uniformBlockBinders[name] = std::move(binder);
  }

private:
  boost::container::flat_map<std::string, std::function<UniformParameter::UniformValueSetter>> m_uniformSetters;
  boost::container::flat_map<std::string, std::function<UniformBlockParameter::BufferBinder>> m_uniformBlockBinders;
  boost::container::flat_map<std::string, std::function<BufferParameter::BufferBinder>> m_bufferBinders;
};
} // namespace render::scene
