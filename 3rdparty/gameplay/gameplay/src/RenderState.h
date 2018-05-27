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
    RenderState(const RenderState&) = delete;

    RenderState& operator=(const RenderState&) = delete;


    class StateBlock
    {
        friend class RenderState;


        friend class Game;


    public:
        StateBlock(const StateBlock& copy) = delete;

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
        void bindNoRestore();

        static void restore(long stateOverrideBits);

        static void enableDepthWrite();

        // States
        bool m_cullFaceEnabled = false;

        bool m_depthTestEnabled = false;

        bool m_depthWriteEnabled = true;

        GLenum m_depthFunction = GL_LESS;

        bool m_blendEnabled = false;

        GLenum m_blendSrc = GL_ONE;

        GLenum m_blendDst = GL_ZERO;

        GLenum m_cullFaceSide = GL_BACK;

        GLenum m_frontFace = GL_CCW;

        long m_bits = 0;

        static std::shared_ptr<StateBlock> m_defaultState;
    };


    const std::shared_ptr<StateBlock>& getStateBlock() const;

    void initStateBlockDefaults();

protected:

    RenderState() = default;

    virtual ~RenderState() = default;

    static void initialize();

    static void finalize();

    void bind();

protected:
    mutable std::shared_ptr<StateBlock> m_state = nullptr;
};
}
