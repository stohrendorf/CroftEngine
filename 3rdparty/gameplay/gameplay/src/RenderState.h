#pragma once

#include "Base.h"
#include "gl/util.h"

#include <boost/optional.hpp>

#include <map>
#include <memory>
#include <vector>


namespace gameplay
{
    class Material;

    class Game;

    class Model;

    class MaterialParameter;

    class Node;


    class RenderState
    {
        friend class Game;

        friend class Material;

        friend class Model;

    public:
        class StateBlock
        {
            friend class RenderState;

            friend class Game;

        public:
            explicit StateBlock();

            ~StateBlock();

            void bind();

            void setBlend(bool enabled);

            void setBlendSrc(GLenum blend);

            void setBlendDst(GLenum blend);

            void setCullFace(bool enabled);

            void setCullFaceSide(GLenum side);

            void setFrontFace(GLenum winding);

            void setDepthTest(bool enabled);

            void setDepthWrite(bool enabled);

            void setDepthFunction(GLenum func);

        private:

            StateBlock(const StateBlock& copy) = delete;

            void bindNoRestore();

            static void restore(long stateOverrideBits);

            static void enableDepthWrite();

            // States
            bool _cullFaceEnabled = false;

            bool _depthTestEnabled = false;

            bool _depthWriteEnabled = true;

            GLenum _depthFunction = GL_LESS;

            bool _blendEnabled = false;

            GLenum _blendSrc = GL_ONE;

            GLenum _blendDst = GL_ZERO;

            GLenum _cullFaceSide = GL_BACK;

            GLenum _frontFace = GL_CCW;

            long _bits = 0;

            static std::shared_ptr<StateBlock> _defaultState;
        };


        const std::shared_ptr<StateBlock>& getStateBlock() const;

        void initStateBlockDefaults();

    protected:

        RenderState() = default;

        virtual ~RenderState() = default;

        static void initialize();

        static void finalize();

        void bind();

    private:

        RenderState(const RenderState&) = delete;

        RenderState& operator=(const RenderState&) = delete;

    protected:
        mutable std::shared_ptr<StateBlock> _state = nullptr;
    };
}
