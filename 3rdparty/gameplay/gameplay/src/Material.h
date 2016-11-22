#pragma once

#include "RenderState.h"
#include "Effect.h"
#include "VertexAttributeBinding.h"


namespace gameplay
{
    /**
     * Defines a material for an object to be rendered.
     *
     * This class encapsulates a set of rendering techniques that can be used to render an
     * object. This class facilitates loading of techniques using specified shaders or
     * material files (.material). When multiple techniques are loaded using a material file,
     * the current technique for an object can be set at runtime.
     */
    class Material : public RenderState
    {
        friend class RenderState;
        friend class Node;
        friend class Model;

    public:
        explicit Material(const std::shared_ptr<ShaderProgram>& shaderProgram);
        ~Material();

        /**
         * Creates a material using the specified vertex and fragment shader.
         *
         * The returned material has a single technique and a single pass for the
         * given effect.
         *
         * @param vshPath Path to the vertex shader file.
         * @param fshPath Path to the fragment shader file.
         * @param defines New-line delimited list of preprocessor defines.
         *
         * @return A new Material.
         * @script{create}
         */
        explicit Material(const std::string& vshPath, const std::string& fshPath, const std::vector<std::string>& defines = {});

        void unbind()
        {
            // If we have a vertex attribute binding, unbind it
            if(_boundVaBinding)
            {
                _boundVaBinding->unbind();
                _boundVaBinding.reset();
            }
        }

        const std::shared_ptr<ShaderProgram>& getShaderProgram() const
        {
            return _shaderProgram;
        }

        void bind(const std::shared_ptr<VertexAttributeBinding>& vaBinding);

    private:

        Material(const Material&) = delete;

        std::shared_ptr<ShaderProgram> _shaderProgram;
        std::shared_ptr<VertexAttributeBinding> _boundVaBinding;
    };
}
