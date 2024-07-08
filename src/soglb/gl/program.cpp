#include "program.h"

#include "bindableresource.h"
#include "glassert.h"

#include <cstdint>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gsl/gsl-lite.hpp>
#include <string>
#include <utility>
#include <vector>

namespace gl
{
Uniform::Uniform(const Program& program, const uint32_t index)
    : LocatableProgramInterface{program, index}
    , m_program{program.getHandle()}
{
  GL_ASSERT(api::getActiveUniforms(program.getHandle(), 1, &index, api::UniformPName::UniformSize, &m_size));
  gsl_Ensures(m_size >= 0);
}

Uniform::Uniform(const Uniform& rhs) noexcept = default;

Uniform& Uniform::operator=(Uniform&& rhs) noexcept
{
  m_size = std::exchange(rhs.m_size, -1);
  m_program = std::exchange(rhs.m_program, InvalidProgram);
  m_value = std::exchange(rhs.m_value, {});
  LocatableProgramInterface::operator=(std::move(rhs));
  return *this;
}

void Uniform::set(const glm::mat3& value)
{
  gsl_Expects(m_program != InvalidProgram);
  if(changeValue(value))
    GL_ASSERT(api::programUniformMatrix3(m_program, getLocation(), 1, false, glm::value_ptr(value)));
}

void Uniform::set(const glm::mat4& value)
{
  gsl_Expects(m_program != InvalidProgram);
  if(changeValue(value))
    GL_ASSERT(api::programUniformMatrix4(m_program, getLocation(), 1, false, glm::value_ptr(value)));
}

void Uniform::set(const std::vector<glm::mat4>& values)
{
  gsl_Expects(m_program != InvalidProgram);
  if(changeValue(values))
    GL_ASSERT(api::programUniformMatrix4(
      m_program,
      getLocation(),
      gsl::narrow<api::core::SizeType>(values.size()),
      false,
      reinterpret_cast<const float*>(values.data()) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
      ));
}

void Uniform::set(const glm::vec2& value)
{
  gsl_Expects(m_program != InvalidProgram);
  if(changeValue(value))
    GL_ASSERT(api::programUniform2(m_program, getLocation(), value.x, value.y));
}

void Uniform::set(const glm::vec3& value)
{
  gsl_Expects(m_program != InvalidProgram);
  if(changeValue(value))
    GL_ASSERT(api::programUniform3(m_program, getLocation(), value.x, value.y, value.z));
}

void Uniform::set(const glm::vec4& value)
{
  gsl_Expects(m_program != InvalidProgram);
  if(changeValue(value))
    GL_ASSERT(api::programUniform4(m_program, getLocation(), value.x, value.y, value.z, value.w));
}

void Uniform::set(const std::vector<glm::vec2>& values)
{
  gsl_Expects(m_program != InvalidProgram);
  if(changeValue(values))
    GL_ASSERT(api::programUniform2(
      m_program,
      getLocation(),
      gsl::narrow<api::core::SizeType>(values.size()),
      reinterpret_cast<const float*>(values.data()) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
      ));
}

void Uniform::set(const std::vector<glm::vec3>& values)
{
  gsl_Expects(m_program != InvalidProgram);
  if(changeValue(values))
    GL_ASSERT(api::programUniform3(
      m_program,
      getLocation(),
      gsl::narrow<api::core::SizeType>(values.size()),
      reinterpret_cast<const float*>(values.data()) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
      ));
}

void Uniform::set(const std::vector<glm::vec4>& values)
{
  gsl_Expects(m_program != InvalidProgram);
  if(changeValue(values))
    GL_ASSERT(api::programUniform4(
      m_program,
      getLocation(),
      gsl::narrow<api::core::SizeType>(values.size()),
      reinterpret_cast<const float*>(values.data()) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
      ));
}

bool Program::getLinkStatus() const
{
  auto success = static_cast<int32_t>(api::Boolean::False);
  GL_ASSERT(api::getProgram(getHandle(), api::ProgramProperty::LinkStatus, &success));
  return success == static_cast<int32_t>(api::Boolean::True);
}

std::string Program::getInfoLog() const
{
  int32_t length = 0;
  GL_ASSERT(api::getProgram(getHandle(), api::ProgramProperty::InfoLogLength, &length));
  if(length == 0)
  {
    length = 4096;
  }
  if(length > 0)
  {
    std::vector<char> infoLog;
    infoLog.resize(length, '\0');
    GL_ASSERT(api::getProgramInfoLog(getHandle(), length, nullptr, infoLog.data()));
    infoLog[length - 1] = '\0';
    std::string result = infoLog.data();
    return result;
  }

  return {};
}

uint32_t Program::getActiveResourceCount(const api::ProgramInterface what) const
{
  int32_t n = 0;
  GL_ASSERT(api::getProgramInterface(getHandle(), what, api::ProgramInterfacePName::ActiveResources, &n));
  return gsl::narrow<uint32_t>(n);
}

std::vector<ProgramInput> Program::getInputs() const
{
  return getInputs<ProgramInput>();
}

std::vector<ProgramOutput> Program::getOutputs() const
{
  return getInputs<ProgramOutput>();
}

std::vector<Uniform> Program::getUniforms() const
{
  return getInputs<Uniform>();
}

std::vector<ShaderStorageBlock> Program::getShaderStorageBlocks() const
{
  return getInputs<ShaderStorageBlock>();
}

std::vector<UniformBlock> Program::getUniformBlocks() const
{
  return getInputs<UniformBlock>();
}
} // namespace gl
