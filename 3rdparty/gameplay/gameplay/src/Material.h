#pragma once

#include "RenderState.h"
#include "Technique.h"


namespace gameplay
{
    /**
     * Defines a material for an object to be rendered.
     *
     * This class encapsulates a set of rendering techniques that can be used to render an
     * object. This class facilitates loading of techniques using specified shaders or
     * material files (.material). When multiple techniques are loaded using a material file,
     * the current technique for an object can be set at runtime.
     *
     * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Materials
     */
    class Material : public RenderState
    {
        friend class Technique;
        friend class Pass;
        friend class RenderState;
        friend class Node;
        friend class Model;

    public:
        explicit Material();
        ~Material();

        /**
         * Pass creation callback function definition.
         */
        typedef std::string (*PassCallback)(const std::shared_ptr<Pass>&, void*);

        /**
         * Creates a material from the specified effect.
         *
         * The returned material has a single technique and a single pass for the
         * given effect.
         *
         * @param shaderProgram Effect for the new material.
         *
         * @return A new Material.
         * @script{create}
         */
        static std::shared_ptr<Material> create(const std::shared_ptr<ShaderProgram>& shaderProgram);

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
        static std::shared_ptr<Material> create(const std::string& vshPath, const std::string& fshPath, const std::vector<std::string>& defines = {});

        /**
         * Returns the technique at the specified index in this material.
         *
         * @return The specified technique.
         */
        const std::shared_ptr<Technique>& getTechnique() const
        {
            return _technique;
        }

        /**
         * @see RenderState::setNodeBinding
         */
        void setNodeBinding(Node* node) override;

    private:

        Material(const Material& m) = delete;

        std::shared_ptr<Technique> _technique;
    };
}
