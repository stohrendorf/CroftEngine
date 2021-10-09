#include "material.h"

#include "bufferparameter.h"
#include "shaderprogram.h"
#include "uniformparameter.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <gl/program.h>
#include <iosfwd>
#include <utility>

namespace render::scene
{
Material::Material(gsl::not_null<std::shared_ptr<ShaderProgram>> shaderProgram)
    : m_shaderProgram{std::move(shaderProgram)}
{
  for(const auto& u : m_shaderProgram->getHandle().getUniforms())
    // cppcheck-suppress useStlAlgorithm
    m_uniforms.emplace_back(std::make_shared<UniformParameter>(u.getName()));
  for(const auto& u : m_shaderProgram->getHandle().getUniformBlocks())
    // cppcheck-suppress useStlAlgorithm
    m_uniformBlocks.emplace_back(std::make_shared<UniformBlockParameter>(u.getName()));
  for(const auto& u : m_shaderProgram->getHandle().getShaderStorageBlocks())
    // cppcheck-suppress useStlAlgorithm
    m_buffers.emplace_back(std::make_shared<BufferParameter>(u.getName()));
}

Material::~Material() = default;

void Material::bind(const Node& node, const Mesh& mesh) const
{
  for(const auto& param : m_uniforms)
  {
    [[maybe_unused]] const auto success = param->bind(node, mesh, m_shaderProgram);
#ifndef NDEBUG
    if(!success)
    {
      BOOST_LOG_TRIVIAL(warning) << "Failed to bind material uniform " << param->getName();
    }
#endif
  }

  for(const auto& param : m_uniformBlocks)
  {
    [[maybe_unused]] const auto success = param->bind(node, mesh, m_shaderProgram);
#ifndef NDEBUG
    if(!success)
    {
      BOOST_LOG_TRIVIAL(warning) << "Failed to bind material uniform block " << param->getName();
    }
#endif
  }

  for(const auto& param : m_buffers)
  {
    [[maybe_unused]] const auto success = param->bind(node, mesh, m_shaderProgram);
#ifndef NDEBUG
    if(!success)
    {
      BOOST_LOG_TRIVIAL(warning) << "Failed to bind material buffer " << param->getName();
    }
#endif
  }

  m_shaderProgram->bind();
}

std::shared_ptr<UniformParameter> Material::tryGetUniform(const std::string& name) const
{
  auto it = std::find_if(m_uniforms.begin(),
                         m_uniforms.end(),
                         [&name](const auto& param)
                         {
                           return param->getName() == name;
                         });
  if(it != m_uniforms.end())
    return *it;

  if(m_shaderProgram->findUniform(name) == nullptr)
    return nullptr;

  auto param = std::make_shared<UniformParameter>(name);
  m_uniforms.emplace_back(param);
  return param;
}

std::shared_ptr<UniformBlockParameter> Material::tryGetUniformBlock(const std::string& name) const
{
  auto it = std::find_if(m_uniformBlocks.begin(),
                         m_uniformBlocks.end(),
                         [&name](const auto& param)
                         {
                           return param->getName() == name;
                         });
  if(it != m_uniformBlocks.end())
    return *it;

  if(m_shaderProgram->findUniformBlock(name) == nullptr)
    return nullptr;
  auto param = std::make_shared<UniformBlockParameter>(name);
  m_uniformBlocks.emplace_back(param);
  return param;
}

std::shared_ptr<BufferParameter> Material::tryGetBuffer(const std::string& name) const
{
  auto it = std::find_if(m_buffers.begin(),
                         m_buffers.end(),
                         [&name](const auto& param)
                         {
                           return param->getName() == name;
                         });
  if(it != m_buffers.end())
    return *it;

  if(m_shaderProgram->findShaderStorageBlock(name) == nullptr)
    return nullptr;
  auto param = std::make_shared<BufferParameter>(name);
  m_buffers.emplace_back(param);
  return param;
}
} // namespace render::scene
