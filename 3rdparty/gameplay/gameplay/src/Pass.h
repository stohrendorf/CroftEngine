#pragma once

#include "RenderState.h"
#include "VertexAttributeBinding.h"


namespace gameplay
{
    class Technique;


    /**
     * Defines a pass for an object to be rendered.
     *
     * This class encapsulates the parameters and logic required to apply a shader
     * to an object to be rendered. This includes specifying both a vertex and fragment
     * shader, as well as any uniforms and vertex attributes to be applied to these.
     */
    class Pass : public RenderState
    {
        friend class Technique;
        friend class Material;
        friend class RenderState;

    public:
        Pass(const std::string& id, const std::shared_ptr<Technique>& technique);
        ~Pass();

        /**
         * Returns the Id of this pass.
         */
        const std::string& getId() const;

        /**
         * Returns the effect for this Pass.
         */
        const std::shared_ptr<ShaderProgram>& getShaderProgram() const;

        /**
         * Binds the render state for this pass.
         *
         * This method should be called before executing any drawing code that should
         * use this pass. When drawing code is complete, the unbind() method should be called.
         */
        void bind(const std::shared_ptr<VertexAttributeBinding>& vaBinding);

        /**
         * Unbinds the render state for this pass.
         *
         * This method should always be called when rendering for a pass is complete, to
         * restore the render state to the state it was in previous to calling bind().
         */
        void unbind();

    private:

        Pass(const Pass& copy) = delete;

        /**
         * Creates a new pass for the given shaders.
         */
        bool initialize(const std::string& vshPath, const std::string& fshPath, const std::vector<std::string>& defines);

        Pass& operator=(const Pass&) = delete;

        std::string _id;
        std::shared_ptr<Technique> _technique;
        std::shared_ptr<ShaderProgram> _shaderProgram;
        std::shared_ptr<VertexAttributeBinding> _boundVaBinding;
    };
}
