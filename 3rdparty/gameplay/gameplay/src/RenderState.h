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
        StateBlock(const StateBlock&) = delete;

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

        static void initDefaults()
        {
            m_currentState.restore( true );
        }

    private:
        void bindNoRestore();

        void restore(bool forceDefaults = false);

        static void enableDepthWrite();

        // States
        boost::optional<bool> m_cullFaceEnabled;

        boost::optional<bool> m_depthTestEnabled;

        boost::optional<bool> m_depthWriteEnabled;

        boost::optional<GLenum> m_depthFunction;

        boost::optional<bool> m_blendEnabled;

        boost::optional<GLenum> m_blendSrc;

        boost::optional<GLenum> m_blendDst;

        boost::optional<GLenum> m_cullFaceSide;

        boost::optional<GLenum> m_frontFace;

        static StateBlock m_currentState;
    };


    StateBlock& getStateBlock();

protected:

    RenderState() = default;

    virtual ~RenderState() = default;

    void bind();

protected:
    mutable StateBlock m_state{};
};
}
