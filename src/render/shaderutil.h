#pragma once

#include <osg/Program>
#include <osgDB/ReadFile>

#include <gsl.h>

namespace render
{
    inline void initDefaultShaders(const gsl::not_null<osg::StateSet*>& stateSet, bool withTexture)
    {
        osg::Shader* vShader = osgDB::readShaderFile(osg::Shader::VERTEX, "def_vertex.glsl");
        osg::Shader* fShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "def_fragment.glsl");

        osg::Program* program = new osg::Program;
        program->addShader(vShader);
        program->addShader(fShader);
        stateSet->setAttribute(program);

        osg::Vec3f lightDir(0., 0.5, 1.);
        lightDir.normalize();
        stateSet->addUniform(new osg::Uniform("ecLightDir", lightDir));

        if(withTexture)
        {
            auto tu = new osg::Uniform(osg::Uniform::SAMPLER_2D, "texture0");
            tu->set(0);
            stateSet->addUniform(tu);
        }
    }
}
