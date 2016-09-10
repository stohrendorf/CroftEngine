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
        Pass(const char* id, const std::shared_ptr<Technique>& technique);
        ~Pass();

        /**
         * Returns the Id of this pass.
         */
        const char* getId() const;

        /**
         * Returns the effect for this Pass.
         */
        const std::shared_ptr<Effect>& getEffect() const;

        /**
         * Sets a vertex attribute binding for this pass.
         *
         * When a mesh binding is set, the VertexAttributeBinding will be automatically
         * bound when the bind() method is called for the pass.
         *
         * @param binding The VertexAttributeBinding to set (or NULL to remove an existing binding).
         */
        void setVertexAttributeBinding(const std::shared_ptr<VertexAttributeBinding>& binding);

        /**
         * Sets a vertex attribute binding for this pass.
         *
         * @return The vertex attribute binding for this pass.
         */
        const std::shared_ptr<VertexAttributeBinding>& getVertexAttributeBinding() const;

        /**
         * Binds the render state for this pass.
         *
         * This method should be called before executing any drawing code that should
         * use this pass. When drawing code is complete, the unbind() method should be called.
         */
        void bind();

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
        bool initialize(const char* vshPath, const char* fshPath, const char* defines);

        Pass& operator=(const Pass&) = delete;

        std::string _id;
        std::shared_ptr<Technique> _technique;
        std::shared_ptr<Effect> _effect;
        std::shared_ptr<VertexAttributeBinding> _vaBinding;
    };
}
