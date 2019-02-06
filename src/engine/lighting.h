#pragma once

#include "util/helpers.h"
#include "core/vec.h"
#include "loader/datatypes.h"

namespace engine
{
struct Lighting
{
    glm::vec3 position = glm::vec3{std::numeric_limits<float>::quiet_NaN()};
    float base = 0;
    float baseDiff = 0;

    void updateDynamic(const core::RoomBoundPosition& pos)
    {
        updateDynamic( -1, pos );
    }

    void updateDynamic(int16_t shade, const core::RoomBoundPosition& pos)
    {
        if( shade >= 0 )
        {
            updateStatic( shade );
            return;
        }

        baseDiff = 0;

        const auto roomAmbient = pos.room->getAmbientBrightness();
        BOOST_ASSERT( roomAmbient >= 0 && roomAmbient <= 1 );

        if( pos.room->lights.empty() )
        {
            base = roomAmbient;
            return;
        }

        float maxBrightness = 0;
        for( const auto& light : pos.room->lights )
        {
            const auto r = util::square( light.radius.cast<float>() / (4096_len).cast<float>() );
            const auto d = util::square(
                    pos.position.distanceTo( light.position ).cast<float>() / (4096_len).cast<float>() );

            const auto lightBrightness = roomAmbient + r * light.getBrightness() / (r + d) / 2;
            if( lightBrightness > maxBrightness )
            {
                maxBrightness = lightBrightness;
                position = light.position.toRenderSystem();
            }
        }

        base = maxBrightness;
        baseDiff = maxBrightness - roomAmbient;
    }

    void updateStatic(int16_t shade)
    {
        base = util::clamp( 2.0f - shade / 8191.0f, 0.0f, 1.0f );
        baseDiff = 0;
        position = glm::vec3{std::numeric_limits<float>::quiet_NaN()};
    }

    void bind(gameplay::Node& node)
    {
        node.addMaterialParameterSetter( "u_baseLight", [this](const gameplay::Node& /*node*/,
                                                               gameplay::gl::Program::ActiveUniform& uniform) {
            uniform.set( base );
        } );
        node.addMaterialParameterSetter( "u_baseLightDiff", [this](const gameplay::Node& /*node*/,
                                                                   gameplay::gl::Program::ActiveUniform& uniform) {
            uniform.set( baseDiff );
        } );
        node.addMaterialParameterSetter( "u_lightPosition", [this](const gameplay::Node& /*node*/,
                                                                   gameplay::gl::Program::ActiveUniform& uniform) {
            uniform.set( position );
        } );
    }
};
}