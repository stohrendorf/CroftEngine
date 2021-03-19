#include "material.h"

#include "node.h"
#include "shaderprogram.h"
#include "uniformparameter.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/log/trivial.hpp>
#include <utility>

namespace render::scene
{
Material::Material(gsl::not_null<std::shared_ptr<ShaderProgram>> shaderProgram)
    : m_shaderProgram{std::move(shaderProgram)}
{
  int32_t samplerIndex = 0;
  for(const auto& u : m_shaderProgram->getHandle().getUniforms(samplerIndex))
    // cppcheck-suppress useStlAlgorithm
    m_uniforms.emplace_back(std::make_shared<UniformParameter>(u.getName()));
  for(const auto& u : m_shaderProgram->getHandle().getUniformBlocks(samplerIndex))
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
    const auto success = param->bind(node, mesh, m_shaderProgram);
#ifndef NDEBUG
    if(!success)
    {
      BOOST_LOG_TRIVIAL(warning) << "Failed to bind material uniform " << param->getName();
    }
#endif
  }

  for(const auto& param : m_uniformBlocks)
  {
    const auto success = param->bind(node, mesh, m_shaderProgram);
#ifndef NDEBUG
    if(!success)
    {
      BOOST_LOG_TRIVIAL(warning) << "Failed to bind material uniform block " << param->getName();
    }
#endif
  }

  for(const auto& param : m_buffers)
  {
    const auto success = param->bind(node, mesh, m_shaderProgram);
#ifndef NDEBUG
    if(!success)
    {
      BOOST_LOG_TRIVIAL(warning) << "Failed to bind material buffer " << param->getName();
    }
#endif
  }

  m_shaderProgram->bind();
}

gsl::not_null<std::shared_ptr<UniformParameter>> Material::getUniform(const std::string& name) const
{
  auto it = std::find_if(
    m_uniforms.begin(), m_uniforms.end(), [&name](const auto& param) { return param->getName() == name; });
  if(it != m_uniforms.end())
    return *it;

  Expects(m_shaderProgram->findUniform(name.c_str()) != nullptr);
  auto param = std::make_shared<UniformParameter>(std::string{name.begin(), name.end()});
  m_uniforms.emplace_back(param);
  return param;
}

gsl::not_null<std::shared_ptr<UniformBlockParameter>> Material::getUniformBlock(const std::string& name) const
{
  auto it = std::find_if(
    m_uniformBlocks.begin(), m_uniformBlocks.end(), [&name](const auto& param) { return param->getName() == name; });
  if(it != m_uniformBlocks.end())
    return *it;

  Expects(m_shaderProgram->findUniformBlock(name.c_str()) != nullptr);
  auto param = std::make_shared<UniformBlockParameter>(std::string{name.begin(), name.end()});
  m_uniformBlocks.emplace_back(param);
  return param;
}

gsl::not_null<std::shared_ptr<BufferParameter>> Material::getBuffer(const std::string& name) const
{
  auto it
    = std::find_if(m_buffers.begin(), m_buffers.end(), [&name](const auto& param) { return param->getName() == name; });
  if(it != m_buffers.end())
    return *it;

  Expects(m_shaderProgram->findShaderStorageBlock(name.c_str()) != nullptr);
  auto param = std::make_shared<BufferParameter>(std::string{name.begin(), name.end()});
  m_buffers.emplace_back(param);
  return param;
}
} // namespace render::scene
