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
        enum Blend : GLenum
        {
            BLEND_ZERO = GL_ZERO,
            BLEND_ONE = GL_ONE,
            BLEND_SRC_COLOR = GL_SRC_COLOR,
            BLEND_ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
            BLEND_DST_COLOR = GL_DST_COLOR,
            BLEND_ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
            BLEND_SRC_ALPHA = GL_SRC_ALPHA,
            BLEND_ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
            BLEND_DST_ALPHA = GL_DST_ALPHA,
            BLEND_ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
            BLEND_CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
            BLEND_ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
            BLEND_SRC_ALPHA_SATURATE = GL_SRC_ALPHA_SATURATE
        };


        enum DepthFunction : GLenum
        {
            DEPTH_NEVER = GL_NEVER,
            DEPTH_LESS = GL_LESS,
            DEPTH_EQUAL = GL_EQUAL,
            DEPTH_LEQUAL = GL_LEQUAL,
            DEPTH_GREATER = GL_GREATER,
            DEPTH_NOTEQUAL = GL_NOTEQUAL,
            DEPTH_GEQUAL = GL_GEQUAL,
            DEPTH_ALWAYS = GL_ALWAYS
        };


        enum CullFaceSide : GLenum
        {
            CULL_FACE_SIDE_BACK = GL_BACK,
            CULL_FACE_SIDE_FRONT = GL_FRONT,
            CULL_FACE_SIDE_FRONT_AND_BACK = GL_FRONT_AND_BACK
        };


        enum FrontFace : GLenum
        {
            FRONT_FACE_CW = GL_CW,
            FRONT_FACE_CCW = GL_CCW
        };


        class StateBlock
        {
            friend class RenderState;

            friend class Game;

        public:
            explicit StateBlock();

            ~StateBlock();

            void bind();

            void setBlend(bool enabled);

            void setBlendSrc(Blend blend);

            void setBlendDst(Blend blend);

            void setCullFace(bool enabled);

            void setCullFaceSide(CullFaceSide side);

            void setFrontFace(FrontFace winding);

            void setDepthTest(bool enabled);

            void setDepthWrite(bool enabled);

            void setDepthFunction(DepthFunction func);

        private:

            StateBlock(const StateBlock& copy) = delete;

            void bindNoRestore();

            static void restore(long stateOverrideBits);

            static void enableDepthWrite();

            // States
            bool _cullFaceEnabled = false;

            bool _depthTestEnabled = false;

            bool _depthWriteEnabled = true;

            DepthFunction _depthFunction = DEPTH_LESS;

            bool _blendEnabled = false;

            Blend _blendSrc = BLEND_ONE;

            Blend _blendDst = BLEND_ZERO;

            CullFaceSide _cullFaceSide = CULL_FACE_SIDE_BACK;

            FrontFace _frontFace = FRONT_FACE_CCW;

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
