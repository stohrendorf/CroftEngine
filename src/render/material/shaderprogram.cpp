#include "shaderprogram.h"

#include <boost/log/trivial.hpp>
#include <gl/api/gl.hpp>
#include <gl/glassert.h>
#include <gl/program.h>
#include <gl/renderstate.h>
#include <utility>

namespace render::material
{
ShaderProgram::~ShaderProgram() = default;

void ShaderProgram::bind() const
{
  GL_ASSERT(gl::api::memoryBarrier(gl::api::MemoryBarrierMask::BufferUpdateBarrierBit
                                   | gl::api::MemoryBarrierMask::ShaderStorageBarrierBit
                                   | gl::api::MemoryBarrierMask::UniformBarrierBit));
  gl::RenderState::getWantedState().setProgram(m_handle.getHandle());
}

void ShaderProgram::initInterface()
{
  for(auto&& input : m_handle.getInputs())
  {
    if(!input.getLocation().has_value())
      continue; // only accept directly accessible uniforms

    BOOST_LOG_TRIVIAL(debug) << "  input " << input.getName() << ", location=" << *input.getLocation();

    m_vertexAttributes.emplace(input.getName(), std::move(input));
  }

  for(auto&& uniform : m_handle.getUniforms())
  {
    if(!uniform.getLocation().has_value())
      continue; // only accept directly accessible uniforms

    BOOST_LOG_TRIVIAL(debug) << "  uniform " << uniform.getName() << ", location=" << *uniform.getLocation();

    m_uniforms.emplace(uniform.getName(), std::move(uniform));
  }

  for(auto&& ub : m_handle.getUniformBlocks())
  {
    gsl_Assert(ub.getBinding().has_value());
    BOOST_LOG_TRIVIAL(debug) << "  uniform block " << ub.getName() << ", binding=" << *ub.getBinding();
    m_uniformBlocks.emplace(ub.getName(), std::move(ub));
  }

  for(auto&& ssb : m_handle.getShaderStorageBlocks())
  {
    gsl_Assert(ssb.getBinding().has_value());
    BOOST_LOG_TRIVIAL(debug) << "  shader storage block " << ssb.getName() << ", binding=" << *ssb.getBinding();
    m_shaderStorageBlocks.emplace(ssb.getName(), std::move(ssb));
  }

  for(auto&& output : m_handle.getOutputs())
  {
    gsl_Assert(output.getLocation());
    BOOST_LOG_TRIVIAL(debug) << "  output " << output.getName() << ", location=" << *output.getLocation();
  }
}
} // namespace render::material
