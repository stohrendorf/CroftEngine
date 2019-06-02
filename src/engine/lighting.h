#pragma once

#include "util/helpers.h"
#include "core/vec.h"
#include "loader/file/datatypes.h"

namespace engine
{
struct Lighting
{
    glm::vec3 position = glm::vec3{std::numeric_limits<float>::quiet_NaN()};
    float ambient = 0;
    float intensity = 0;

    void updateDynamic(int16_t shade, const core::RoomBoundPosition& pos)
    {
        if( shade >= 0 )
        {
            updateStatic( shade );
            return;
        }

        const auto roomAmbient = pos.room->getAmbientBrightness();
        BOOST_ASSERT( roomAmbient >= 0 && roomAmbient <= 1 );
        ambient = roomAmbient;
        intensity = 0;

        if( pos.room->lights.empty() )
        {
            return;
        }

        for( const auto& light : pos.room->lights )
        {
            const auto fadeDistance = util::square( light.fadeDistance.retype_as<core::LengthF>() / 4096.0_len );
            const auto d = util::square(
                    pos.position.distanceTo( light.position ).retype_as<core::LengthF>() / 4096.0_len );

            // http://www-f9.ijs.si/~matevz/docs/PovRay/pov274.htm
            // 1 / ( 1 + (d/fade_distance) ^ fade_power );
            // assuming fade_power = 1, multiply numerator and denominator with fade_distance (identity transform):
            // fade_distance / ( fade_distance + d )
            const auto lightBrightness = roomAmbient + light.getBrightness() * fadeDistance / (fadeDistance + d);
            if( lightBrightness > intensity )
            {
                intensity = lightBrightness;
                position = light.position.toRenderSystem();
            }
        }
    }

    void updateStatic(int16_t shade)
    {
        ambient = 1.0f - shade / 8191.0f;
        intensity = 0;
        position = glm::vec3{std::numeric_limits<float>::quiet_NaN()};
    }

    void bind(render::scene::Node& node)
    {
        node.addMaterialParameterSetter( "u_lightAmbient", [this](const render::scene::Node& /*node*/,
                                                               render::gl::Program::ActiveUniform& uniform) {
            uniform.set( ambient );
        } );
        node.addMaterialParameterSetter( "u_lightIntensity", [this](const render::scene::Node& /*node*/,
                                                                   render::gl::Program::ActiveUniform& uniform) {
            uniform.set( intensity );
        } );
        node.addMaterialParameterSetter( "u_lightPosition", [this](const render::scene::Node& /*node*/,
                                                                   render::gl::Program::ActiveUniform& uniform) {
            uniform.set( position );
        } );
    }
};
}