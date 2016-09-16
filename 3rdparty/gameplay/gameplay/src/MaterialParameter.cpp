#include "Base.h"
#include "MaterialParameter.h"
#include "Node.h"


namespace gameplay
{
    MaterialParameter::MaterialParameter(const char* name)
        : _type(MaterialParameter::NONE)
        , _count(1)
        , _dynamic(false)
        , _name(name ? name : "")
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

        memset(&_value, 0, sizeof(_value));
        _type = MaterialParameter::NONE;
    }


    const char* MaterialParameter::getName() const
    {
        return _name.c_str();
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


    void MaterialParameter::setValue(const float* values, unsigned int count)
    {
        clearValue();

        _value = const_cast<float*>(values);
        _count = count;
        _type = MaterialParameter::FLOAT_ARRAY;
    }


    void MaterialParameter::setValue(const int* values, unsigned int count)
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


    void MaterialParameter::setValue(const Vector2* values, unsigned int count)
    {
        GP_ASSERT(values);
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


    void MaterialParameter::setValue(const Vector3* values, unsigned int count)
    {
        GP_ASSERT(values);
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


    void MaterialParameter::setValue(const Vector4* values, unsigned int count)
    {
        GP_ASSERT(values);
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


    void MaterialParameter::setValue(const Matrix* values, unsigned int count)
    {
        GP_ASSERT(values);
        clearValue();

        _value = const_cast<float*>(values[0].m);
        _count = count;
        _type = MaterialParameter::MATRIX;
    }


    void MaterialParameter::setValue(const std::shared_ptr<Texture::Sampler>& sampler)
    {
        GP_ASSERT(sampler);
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


    void MaterialParameter::setFloatArray(const float* values, unsigned int count, bool copy)
    {
        GP_ASSERT(values);
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


    void MaterialParameter::setIntArray(const int* values, unsigned int count, bool copy)
    {
        GP_ASSERT(values);
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


    void MaterialParameter::setVector2Array(const Vector2* values, unsigned int count, bool copy)
    {
        GP_ASSERT(values);
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


    void MaterialParameter::setVector3Array(const Vector3* values, unsigned int count, bool copy)
    {
        GP_ASSERT(values);
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


    void MaterialParameter::setVector4Array(const Vector4* values, unsigned int count, bool copy)
    {
        GP_ASSERT(values);
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


    void MaterialParameter::setMatrixArray(const Matrix* values, unsigned int count, bool copy)
    {
        GP_ASSERT(values);
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
        GP_ASSERT(shaderProgram);

        // If we had a Uniform cached that is not from the passed in effect,
        // we need to update our uniform to point to the new effect's uniform.
        if( !_uniform || _uniform->getEffect() != shaderProgram )
        {
            _uniform = shaderProgram->getUniform(_name.c_str());

            if( !_uniform )
            {
                if( (_loggerDirtyBits & UNIFORM_NOT_FOUND) == 0 )
                {
                    // This parameter was not found in the specified effect, so do nothing.
                    GP_WARN("Material parameter for uniform '%s' not found in effect: '%s'.", _name.c_str(), shaderProgram->getId().c_str());
                    _loggerDirtyBits |= UNIFORM_NOT_FOUND;
                }
                return;
            }
        }

        switch( _type )
        {
            case MaterialParameter::FLOAT:
                shaderProgram->setValue(_uniform, boost::get<float>(_value));
                break;
            case MaterialParameter::FLOAT_ARRAY:
                shaderProgram->setValue(_uniform, boost::get<float*>(_value), _count);
                break;
            case MaterialParameter::INT:
                shaderProgram->setValue(_uniform, boost::get<int>(_value));
                break;
            case MaterialParameter::INT_ARRAY:
                shaderProgram->setValue(_uniform, boost::get<int*>(_value), _count);
                break;
            case MaterialParameter::VECTOR2:
                shaderProgram->setValue(_uniform, reinterpret_cast<Vector2*>(boost::get<float*>(_value)), _count);
                break;
            case MaterialParameter::VECTOR3:
                shaderProgram->setValue(_uniform, reinterpret_cast<Vector3*>(boost::get<float*>(_value)), _count);
                break;
            case MaterialParameter::VECTOR4:
                shaderProgram->setValue(_uniform, reinterpret_cast<Vector4*>(boost::get<float*>(_value)), _count);
                break;
            case MaterialParameter::MATRIX:
                shaderProgram->setValue(_uniform, reinterpret_cast<Matrix*>(boost::get<float*>(_value)), _count);
                break;
            case MaterialParameter::SAMPLER:
                shaderProgram->setValue(_uniform, boost::get<std::shared_ptr<Texture::Sampler>>(_value));
                break;
            case MaterialParameter::SAMPLER_ARRAY:
                shaderProgram->setValue(_uniform, boost::get<std::vector<std::shared_ptr<Texture::Sampler>>>(_value));
                break;
            case MaterialParameter::METHOD:
                if(boost::get<std::shared_ptr<MethodBinding>>(_value))
                    boost::get<std::shared_ptr<MethodBinding>>(_value)->setValue(shaderProgram);
                break;
            default:
            {
                if( (_loggerDirtyBits & PARAMETER_VALUE_NOT_SET) == 0 )
                {
                    GP_WARN("Material parameter value not set for: '%s' in effect: '%s'.", _name.c_str(), shaderProgram->getId().c_str());
                    _loggerDirtyBits |= PARAMETER_VALUE_NOT_SET;
                }
                break;
            }
        }
    }


    void MaterialParameter::bindValue(Node* node, const char* binding)
    {
        GP_ASSERT(binding);

        if( strcmp(binding, "&Node::getBackVector") == 0 )
        {
            bindValue<Node, Vector3>(node, &Node::getBackVector);
        }
        else if( strcmp(binding, "&Node::getDownVector") == 0 )
        {
            bindValue<Node, Vector3>(node, &Node::getDownVector);
        }
        else if( strcmp(binding, "&Node::getTranslationWorld") == 0 )
        {
            bindValue<Node, Vector3>(node, &Node::getTranslationWorld);
        }
        else if( strcmp(binding, "&Node::getTranslationView") == 0 )
        {
            bindValue<Node, Vector3>(node, &Node::getTranslationView);
        }
        else if( strcmp(binding, "&Node::getForwardVector") == 0 )
        {
            bindValue<Node, Vector3>(node, &Node::getForwardVector);
        }
        else if( strcmp(binding, "&Node::getForwardVectorWorld") == 0 )
        {
            bindValue<Node, Vector3>(node, &Node::getForwardVectorWorld);
        }
        else if( strcmp(binding, "&Node::getForwardVectorView") == 0 )
        {
            bindValue<Node, Vector3>(node, &Node::getForwardVectorView);
        }
        else if( strcmp(binding, "&Node::getLeftVector") == 0 )
        {
            bindValue<Node, Vector3>(node, &Node::getLeftVector);
        }
        else if( strcmp(binding, "&Node::getRightVector") == 0 )
        {
            bindValue<Node, Vector3>(node, &Node::getRightVector);
        }
        else if( strcmp(binding, "&Node::getRightVectorWorld") == 0 )
        {
            bindValue<Node, Vector3>(node, &Node::getRightVectorWorld);
        }
        else if( strcmp(binding, "&Node::getUpVector") == 0 )
        {
            bindValue<Node, Vector3>(node, &Node::getUpVector);
        }
        else if( strcmp(binding, "&Node::getUpVectorWorld") == 0 )
        {
            bindValue<Node, Vector3>(node, &Node::getUpVectorWorld);
        }
        else if( strcmp(binding, "&Node::getActiveCameraTranslationWorld") == 0 )
        {
            bindValue<Node, Vector3>(node, &Node::getActiveCameraTranslationWorld);
        }
        else if( strcmp(binding, "&Node::getActiveCameraTranslationView") == 0 )
        {
            bindValue<Node, Vector3>(node, &Node::getActiveCameraTranslationView);
        }
        else if( strcmp(binding, "&Node::getScaleX") == 0 )
        {
            bindValue<Node, float>(node, &Node::getScaleX);
        }
        else if( strcmp(binding, "&Node::getScaleY") == 0 )
        {
            bindValue<Node, float>(node, &Node::getScaleY);
        }
        else if( strcmp(binding, "&Node::getScaleZ") == 0 )
        {
            bindValue<Node, float>(node, &Node::getScaleZ);
        }
        else if( strcmp(binding, "&Node::getTranslationX") == 0 )
        {
            bindValue<Node, float>(node, &Node::getTranslationX);
        }
        else if( strcmp(binding, "&Node::getTranslationY") == 0 )
        {
            bindValue<Node, float>(node, &Node::getTranslationY);
        }
        else if( strcmp(binding, "&Node::getTranslationZ") == 0 )
        {
            bindValue<Node, float>(node, &Node::getTranslationZ);
        }
        else
        {
            GP_WARN("Unsupported material parameter binding '%s'.", binding);
        }
    }


    MaterialParameter::MethodBinding::MethodBinding(MaterialParameter* param)
        : _parameter(param)
        , _autoBinding(false)
    {
    }
}
