#include "Base.h"
#include "MaterialParameter.h"
#include "Node.h"

#include <boost/log/trivial.hpp>

namespace gameplay
{
    MaterialParameter::MaterialParameter(const std::string& name)
        : _type(MaterialParameter::NONE)
        , _count(1)
        , _dynamic(false)
        , _name(name)
        , _uniform(nullptr)
        , _loggerDirtyBits(0)
    {
        clearValue();
    }


    MaterialParameter::~MaterialParameter()
    {
        clearValue();
    }


    void MaterialParameter::clearValue()
    {
        // Release parameters
        switch( _type )
        {
            case MaterialParameter::SAMPLER:
                boost::get<std::shared_ptr<Texture::Sampler>>(_value).reset();
                break;
            case MaterialParameter::SAMPLER_ARRAY:
                boost::get<std::vector<std::shared_ptr<Texture::Sampler>>>(_value).clear();
                break;
            default:
                // Ignore all other cases.
                break;
        }

        // Free dynamic data
        if( _dynamic )
        {
            switch( _type )
            {
                case MaterialParameter::FLOAT:
                case MaterialParameter::FLOAT_ARRAY:
                case MaterialParameter::VECTOR2:
                case MaterialParameter::VECTOR3:
                case MaterialParameter::VECTOR4:
                case MaterialParameter::MATRIX:
                    SAFE_DELETE_ARRAY(boost::get<float*>(_value));
                    break;
                case MaterialParameter::INT:
                case MaterialParameter::INT_ARRAY:
                    SAFE_DELETE_ARRAY(boost::get<int*>(_value));
                    break;
                case MaterialParameter::METHOD:
                    boost::get<std::shared_ptr<MethodBinding>>(_value).reset();
                    break;
                case MaterialParameter::SAMPLER_ARRAY:
                    boost::get<std::vector<std::shared_ptr<Texture::Sampler>>>(_value).clear();
                    break;
                default:
                    // Ignore all other cases.
                    break;
            }

            _dynamic = false;
            _count = 1;
        }

        _value = nullptr;
        _type = MaterialParameter::NONE;
    }


    const std::string& MaterialParameter::getName() const
    {
        return _name;
    }


    std::shared_ptr<Texture::Sampler> MaterialParameter::getSampler(size_t index) const
    {
        if( _type == MaterialParameter::SAMPLER )
            return boost::get<std::shared_ptr<Texture::Sampler>>(_value);
        if( _type == MaterialParameter::SAMPLER_ARRAY && index < _count )
            return boost::get<std::vector<std::shared_ptr<Texture::Sampler>>>(_value)[index];
        return nullptr;
    }


    void MaterialParameter::setValue(float value)
    {
        clearValue();

        _value = value;
        _type = MaterialParameter::FLOAT;
    }


    void MaterialParameter::setValue(int value)
    {
        clearValue();

        _value = value;
        _type = MaterialParameter::INT;
    }


    void MaterialParameter::setValue(const float* values, size_t count)
    {
        clearValue();

        _value = const_cast<float*>(values);
        _count = count;
        _type = MaterialParameter::FLOAT_ARRAY;
    }


    void MaterialParameter::setValue(const int* values, size_t count)
    {
        clearValue();

        _value = const_cast<int*>(values);
        _count = count;
        _type = MaterialParameter::INT_ARRAY;
    }


    void MaterialParameter::setValue(const Vector2& value)
    {
        clearValue();

        // Copy data by-value into a dynamic array.
        float* array = new float[2];
        memcpy(array, &value.x, sizeof(float) * 2);

        _value = array;
        _dynamic = true;
        _count = 1;
        _type = MaterialParameter::VECTOR2;
    }


    void MaterialParameter::setValue(const Vector2* values, size_t count)
    {
        BOOST_ASSERT(values);
        clearValue();

        _value = const_cast<float*>(&values[0].x);
        _count = count;
        _type = MaterialParameter::VECTOR2;
    }


    void MaterialParameter::setValue(const Vector3& value)
    {
        clearValue();

        // Copy data by-value into a dynamic array.
        float* array = new float[3];
        memcpy(array, &value.x, sizeof(float) * 3);

        _value = array;
        _dynamic = true;
        _count = 1;
        _type = MaterialParameter::VECTOR3;
    }


    void MaterialParameter::setValue(const Vector3* values, size_t count)
    {
        BOOST_ASSERT(values);
        clearValue();

        _value = const_cast<float*>(&values[0].x);
        _count = count;
        _type = MaterialParameter::VECTOR3;
    }


    void MaterialParameter::setValue(const Vector4& value)
    {
        clearValue();

        // Copy data by-value into a dynamic array.
        float* array = new float[4];
        memcpy(array, &value.x, sizeof(float) * 4);

        _value = array;
        _dynamic = true;
        _count = 1;
        _type = MaterialParameter::VECTOR4;
    }


    void MaterialParameter::setValue(const Vector4* values, size_t count)
    {
        BOOST_ASSERT(values);
        clearValue();

        _value = const_cast<float*>(&values[0].x);
        _count = count;
        _type = MaterialParameter::VECTOR4;
    }


    void MaterialParameter::setValue(const Matrix& value)
    {
        // If this parameter is already storing a single dynamic matrix, no need to clear it.
        if( !(_dynamic && _count == 1 && _type == MaterialParameter::MATRIX && boost::get<float*>(_value) != nullptr) )
        {
            clearValue();

            // Allocate a new dynamic matrix.
            _value = new float[16];
        }

        memcpy(boost::get<float*>(_value), value.m, sizeof(float) * 16);

        _dynamic = true;
        _count = 1;
        _type = MaterialParameter::MATRIX;
    }


    void MaterialParameter::setValue(const Matrix* values, size_t count)
    {
        BOOST_ASSERT(values);
        clearValue();

        _value = const_cast<float*>(values[0].m);
        _count = count;
        _type = MaterialParameter::MATRIX;
    }


    void MaterialParameter::setValue(const std::shared_ptr<Texture::Sampler>& sampler)
    {
        BOOST_ASSERT(sampler);
        clearValue();

        _value = sampler;
        _type = MaterialParameter::SAMPLER;
    }


    void MaterialParameter::setValue(const std::vector<std::shared_ptr<Texture::Sampler>>& samplers)
    {
        clearValue();

        _value = samplers;
        _count = samplers.size();
        _type = MaterialParameter::SAMPLER_ARRAY;
    }


    void MaterialParameter::setFloat(float value)
    {
        setValue(value);
    }


    void MaterialParameter::setFloatArray(const float* values, size_t count, bool copy)
    {
        BOOST_ASSERT(values);
        clearValue();

        if( copy )
        {
            _value = new float[count];
            memcpy(boost::get<float*>(_value), values, sizeof(float) * count);
            _dynamic = true;
        }
        else
        {
            _value = const_cast<float*>(values);
        }

        _count = count;
        _type = MaterialParameter::FLOAT_ARRAY;
    }


    void MaterialParameter::setInt(int value)
    {
        setValue(value);
    }


    void MaterialParameter::setIntArray(const int* values, size_t count, bool copy)
    {
        BOOST_ASSERT(values);
        clearValue();

        if( copy )
        {
            _value = new int[count];
            memcpy(boost::get<int*>(_value), values, sizeof(int) * count);
            _dynamic = true;
        }
        else
        {
            _value = const_cast<int*>(values);
        }

        _count = count;
        _type = MaterialParameter::INT_ARRAY;
    }


    void MaterialParameter::setVector2(const Vector2& value)
    {
        setValue(value);
    }


    void MaterialParameter::setVector2Array(const Vector2* values, size_t count, bool copy)
    {
        BOOST_ASSERT(values);
        clearValue();

        if( copy )
        {
            _value = new float[2 * count];
            memcpy(boost::get<float*>(_value), &values[0].x, sizeof(float) * 2 * count);
            _dynamic = true;
        }
        else
        {
            _value = const_cast<float*>(&values[0].x);
        }

        _count = count;
        _type = MaterialParameter::VECTOR2;
    }


    void MaterialParameter::setVector3(const Vector3& value)
    {
        setValue(value);
    }


    void MaterialParameter::setVector3Array(const Vector3* values, size_t count, bool copy)
    {
        BOOST_ASSERT(values);
        clearValue();

        if( copy )
        {
            _value = new float[3 * count];
            memcpy(boost::get<float*>(_value), &values[0].x, sizeof(float) * 3 * count);
            _dynamic = true;
        }
        else
        {
            _value = const_cast<float*>(&values[0].x);
        }

        _count = count;
        _type = MaterialParameter::VECTOR3;
    }


    void MaterialParameter::setVector4(const Vector4& value)
    {
        setValue(value);
    }


    void MaterialParameter::setVector4Array(const Vector4* values, size_t count, bool copy)
    {
        BOOST_ASSERT(values);
        clearValue();

        if( copy )
        {
            _value = new float[4 * count];
            memcpy(boost::get<float*>(_value), &values[0].x, sizeof(float) * 4 * count);
            _dynamic = true;
        }
        else
        {
            _value = const_cast<float*>(&values[0].x);
        }

        _count = count;
        _type = MaterialParameter::VECTOR4;
    }


    void MaterialParameter::setMatrix(const Matrix& value)
    {
        setValue(value);
    }


    void MaterialParameter::setMatrixArray(const Matrix* values, size_t count, bool copy)
    {
        BOOST_ASSERT(values);
        clearValue();

        if( copy )
        {
            _value = new float[16 * count];
            memcpy(boost::get<float*>(_value), values[0].m, sizeof(float) * 16 * count);
            _dynamic = true;
        }
        else
        {
            _value = &const_cast<Matrix&>(values[0]).m[0];
        }

        _count = count;
        _type = MaterialParameter::MATRIX;
    }


    void MaterialParameter::setSampler(const std::shared_ptr<Texture::Sampler>& value)
    {
        setValue(value);
    }


    void MaterialParameter::setSamplerArray(const std::vector<std::shared_ptr<Texture::Sampler>>& values)
    {
        clearValue();

        _value = values;
        _count = values.size();
        _type = MaterialParameter::SAMPLER_ARRAY;
    }


    void MaterialParameter::bind(const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        BOOST_ASSERT(shaderProgram);

        // If we had a Uniform cached that is not from the passed in effect,
        // we need to update our uniform to point to the new effect's uniform.
        if( !_uniform || _uniform->getShaderProgram() != shaderProgram )
        {
            _uniform = shaderProgram->getUniform(_name);

            if( !_uniform )
            {
                if( (_loggerDirtyBits & UNIFORM_NOT_FOUND) == 0 )
                {
                    // This parameter was not found in the specified effect, so do nothing.
                    BOOST_LOG_TRIVIAL(warning) <<  "Material parameter for uniform '" << _name << "' not found in effect: '" << shaderProgram->getId() << "'.";
                    _loggerDirtyBits |= UNIFORM_NOT_FOUND;
                }
                return;
            }
        }

        switch( _type )
        {
            case MaterialParameter::FLOAT:
                shaderProgram->setValue(*_uniform, boost::get<float>(_value));
                break;
            case MaterialParameter::FLOAT_ARRAY:
                shaderProgram->setValue(*_uniform, boost::get<float*>(_value), _count);
                break;
            case MaterialParameter::INT:
                shaderProgram->setValue(*_uniform, boost::get<int>(_value));
                break;
            case MaterialParameter::INT_ARRAY:
                shaderProgram->setValue(*_uniform, boost::get<int*>(_value), _count);
                break;
            case MaterialParameter::VECTOR2:
                shaderProgram->setValue(*_uniform, reinterpret_cast<Vector2*>(boost::get<float*>(_value)), _count);
                break;
            case MaterialParameter::VECTOR3:
                shaderProgram->setValue(*_uniform, reinterpret_cast<Vector3*>(boost::get<float*>(_value)), _count);
                break;
            case MaterialParameter::VECTOR4:
                shaderProgram->setValue(*_uniform, reinterpret_cast<Vector4*>(boost::get<float*>(_value)), _count);
                break;
            case MaterialParameter::MATRIX:
                shaderProgram->setValue(*_uniform, reinterpret_cast<Matrix*>(boost::get<float*>(_value)), _count);
                break;
            case MaterialParameter::SAMPLER:
                shaderProgram->setValue(*_uniform, boost::get<std::shared_ptr<Texture::Sampler>>(_value));
                break;
            case MaterialParameter::SAMPLER_ARRAY:
                shaderProgram->setValue(*_uniform, boost::get<std::vector<std::shared_ptr<Texture::Sampler>>>(_value));
                break;
            case MaterialParameter::METHOD:
                if(boost::get<std::shared_ptr<MethodBinding>>(_value))
                    boost::get<std::shared_ptr<MethodBinding>>(_value)->setValue(shaderProgram);
                break;
            default:
            {
                if( (_loggerDirtyBits & PARAMETER_VALUE_NOT_SET) == 0 )
                {
                    BOOST_LOG_TRIVIAL(warning) << "Material parameter value not set for: '" << _name << "' in effect: '" << shaderProgram->getId() << "'.";
                    _loggerDirtyBits |= PARAMETER_VALUE_NOT_SET;
                }
                break;
            }
        }
    }


    MaterialParameter::MethodBinding::MethodBinding(MaterialParameter* param)
        : _parameter(param)
        , _autoBinding(false)
    {
    }
}
