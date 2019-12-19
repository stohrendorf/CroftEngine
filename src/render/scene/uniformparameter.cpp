#include "uniformparameter.h"

#include "camera.h"
#include "node.h"
#include "scene.h"

namespace render::scene
{
bool UniformParameter::bind(const Node& node, const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram)
{
  const auto setter = node.findUniformSetter(getName());
  if(!m_valueSetter && setter == nullptr)
  {
    // don't have an explicit setter present on material or node level, assuming it's set on shader level
    return true;
  }

  const auto uniform = findUniform(shaderProgram);
  if(uniform == nullptr)
    return false;

  if(setter != nullptr)
    (*setter)(node, *uniform);
  else
    (*m_valueSetter)(node, *uniform);

  return true;
}

void UniformParameter::bindModelMatrix()
{
  m_valueSetter = [](const Node& node, gl::ProgramUniform& uniform) { uniform.set(node.getModelMatrix()); };
}

void UniformParameter::bindViewMatrix()
{
  m_valueSetter = [](const Node& node, gl::ProgramUniform& uniform) { uniform.set(node.getViewMatrix()); };
}

void UniformParameter::bindModelViewMatrix()
{
  m_valueSetter = [](const Node& node, gl::ProgramUniform& uniform) { uniform.set(node.getModelViewMatrix()); };
}

void UniformParameter::bindProjectionMatrix()
{
  m_valueSetter = [](const Node& node, gl::ProgramUniform& uniform) { uniform.set(node.getProjectionMatrix()); };
}

void UniformParameter::bindLightModelViewProjection()
{
  m_valueSetter = [](const render::scene::Node& node, render::gl::ProgramUniform& uniform) {
    const auto camera = node.getScene()->getActiveCamera();
    const auto pos = camera->getPosition();
    const auto fwd = glm::normalize(camera->getFrontVector());
    static constexpr float VolumeSize = 20.0f * 1024;
    const auto ctr = pos + fwd * VolumeSize / 2.0f;
    const auto eye = ctr + glm::vec3{0.0f, 1.0f, 0.0f} * VolumeSize / 2.0f - glm::vec3{VolumeSize / 5, 0.0f, 0.0f};
    const auto lightView = glm::lookAt(eye, ctr, glm::vec3{0.0f, 1.0f, 0.0f});
    const auto lightProjection
      = glm::ortho(-VolumeSize / 2, VolumeSize / 2, VolumeSize / 2, -VolumeSize / 2, 0.0f, VolumeSize);
    uniform.set(lightProjection * lightView * node.getModelMatrix());
  };
}
} // namespace render::scene
