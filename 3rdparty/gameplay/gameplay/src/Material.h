#pragma once

#include "RenderState.h"
#include "ShaderProgram.h"

#include <gsl/gsl>

namespace gameplay
{
class MaterialParameter;


class Node;


class Material : public RenderState
{
public:
    explicit Material(const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram);

    ~Material();

    explicit Material(const std::string& vshPath, const std::string& fshPath,
                      const std::vector<std::string>& defines = {});

    const gsl::not_null<std::shared_ptr<ShaderProgram>>& getShaderProgram() const
    {
        return m_shaderProgram;
    }

    void bind(const Node& node);

    gsl::not_null<std::shared_ptr<MaterialParameter>> getParameter(const std::string& name) const;

private:

    Material(const Material&) = delete;

    gsl::not_null<std::shared_ptr<ShaderProgram>> m_shaderProgram;

    mutable std::vector<gsl::not_null<std::shared_ptr<MaterialParameter>>> m_parameters;
};
}
