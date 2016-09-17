#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Texture.h"
#include "Effect.h"
#include <boost/variant.hpp>

namespace gameplay
{
    class Node;


    /**
     * Defines a material parameter.
     *
     * This class represents a parameter that can be set for a material.
     * The methods in this class provide a mechanism to set parameters
     * of all supported types. Some types support setting by value,
     * while others only support setting by reference/pointer.
     *
     * Setting a parameter by reference/pointer provides the ability to
     * pass an array of values as well as a convenient way to support
     * auto-binding of values to a material parameter. For example, by
     * setting the parameter value to a pointer to a Matrix, any changes
     * to the Matrix will automatically be reflected in the technique the
     * next time the parameter is applied to the render state.
     *
     * Note that for parameter values to arrays or pointers, the
     * MaterialParameter will keep a long-lived reference to the passed
     * in array/pointer. Therefore, you must ensure that the pointers
     * you pass in are valid for the lifetime of the MaterialParameter
     * object.
     */
    class MaterialParameter
    {
        friend class RenderState;

    public:
        explicit MaterialParameter(const std::string& name);
        ~MaterialParameter();

        /**
         * Returns the name of this material parameter.
         */
        const std::string& getName() const;

        /**
         * Returns the texture sampler or NULL if this MaterialParameter is not a sampler type.
         *
         * @param index Index of the sampler (if the parameter is a sampler array),
         *      or zero if it is a single sampler value.
         *
         * @return The texture sampler or NULL if this MaterialParameter is not a sampler type.
         */
        std::shared_ptr<Texture::Sampler> getSampler(size_t index = 0) const;

        /**
         * Sets the value of this parameter to a float value.
         */
        void setValue(float value);

        /**
         * Sets the value of this parameter to an integer value.
         */
        void setValue(int value);

        /**
         * Stores a pointer/array of float values in this parameter.
         */
        void setValue(const float* values, size_t count = 1);

        /**
         * Stores a pointer/array of integer values in this parameter.
         */
        void setValue(const int* values, size_t count = 1);

        /**
         * Stores a copy of the specified Vector2 value in this parameter.
         */
        void setValue(const Vector2& value);

        /**
         * Stores a pointer/array of Vector2 values in this parameter.
         */
        void setValue(const Vector2* values, size_t count = 1);

        /**
         * Stores a copy of the specified Vector3 value in this parameter.
         */
        void setValue(const Vector3& value);

        /**
         * Stores a pointer/array of Vector3 values in this parameter.
         */
        void setValue(const Vector3* values, size_t count = 1);

        /**
         * Stores a copy of the specified Vector4 value in this parameter.
         */
        void setValue(const Vector4& value);

        /**
         * Stores a pointer/array of Vector4 values in this parameter.
         */
        void setValue(const Vector4* values, size_t count = 1);

        /**
         * Stores a copy of the specified Matrix value in this parameter.
         */
        void setValue(const Matrix& value);

        /**
         * Stores a pointer/array of Matrix values in this parameter.
         */
        void setValue(const Matrix* values, size_t count = 1);

        /**
         * Sets the value of this parameter to the specified texture sampler.
         */
        void setValue(const std::shared_ptr<Texture::Sampler>& sampler);

        /**
         * Sets the value of this parameter to the specified texture sampler array.
         *
         * @script{ignore}
         */
        void setValue(const std::vector<std::shared_ptr<Texture::Sampler>>& samplers);

        /**
         * Stores a float value in this parameter.
         *
         * @param value The value to set.
         */
        void setFloat(float value);

        /**
         * Stores an array of float values in this parameter.
         *
         * @param values The array of values.
         * @param count The number of values in the array.
         * @param copy True to make a copy of the array in the material parameter, or false
         *      to point to the passed in array/pointer (which must be valid for the lifetime
         *      of the MaterialParameter).
         */
        void setFloatArray(const float* values, size_t count, bool copy = false);

        /**
         * Stores an integer value in this parameter.
         *
         * @param value The value to set.
         */
        void setInt(int value);

        /**
         * Stores an array of integer values in this parameter.
         */
        void setIntArray(const int* values, size_t count, bool copy = false);

        /**
         * Stores a Vector2 value in this parameter.
         *
         * @param value The value to set.
         */
        void setVector2(const Vector2& value);

        /**
         * Stores an array of Vector2 values in this parameter.
         *
         * @param values The array of values.
         * @param count The number of values in the array.
         * @param copy True to make a copy of the array in the material parameter, or false
         *      to point to the passed in array/pointer (which must be valid for the lifetime
         *      of the MaterialParameter).
         */
        void setVector2Array(const Vector2* values, size_t count, bool copy = false);

        /**
         * Stores a Vector3 value in this parameter.
         *
         * @param value The value to set.
         */
        void setVector3(const Vector3& value);

        /**
         * Stores an array of Vector3 values in this parameter.
         */
        void setVector3Array(const Vector3* values, size_t count, bool copy = false);

        /**
         * Stores a Vector4 value in this parameter.
         *
         * @param value The value to set.
         */
        void setVector4(const Vector4& value);

        /**
         * Stores an array of Vector4 values in this parameter.
         *
         * @param values The array of values.
         * @param count The number of values in the array.
         * @param copy True to make a copy of the array in the material parameter, or false
         *      to point to the passed in array/pointer (which must be valid for the lifetime
         *      of the MaterialParameter).
         */
        void setVector4Array(const Vector4* values, size_t count, bool copy = false);

        /**
         * Stores a Matrix value in this parameter.
         *
         * @param value The value to set.
         */
        void setMatrix(const Matrix& value);

        /**
         * Stores an array of Matrix values in this parameter.
         *
         * @param values The array of values.
         * @param count The number of values in the array.
         * @param copy True to make a copy of the array in the material parameter, or false
         *      to point to the passed in array/pointer (which must be valid for the lifetime
         *      of the MaterialParameter).
         */
        void setMatrixArray(const Matrix* values, size_t count, bool copy = false);

        /**
         * Stores a Sampler value in this parameter.
         *
         * @param value The value to set.
         */
        void setSampler(const std::shared_ptr<Texture::Sampler>& value);

        /**
         * Stores an array of Sampler values in this parameter.
         *
         * @param values The array of values.
         * @script{ignore}
         */
        void setSamplerArray(const std::vector<std::shared_ptr<Texture::Sampler>>& values);

        /**
         * Binds the return value of a class method to this material parameter.
         *
         * This method enables binding of arbitrary class methods to a material
         * parameter. This is useful when you want to set a material parameter
         * to a variable that is frequently changing (such as a world matrix).
         *
         * By binding a method pointer, the method will be called automatically
         * to retrieve the updated parameter value each time the material is bound
         * for rendering.
         *
         * @param classInstance The instance of the class containing the member method to bind.
         * @param valueMethod A pointer to the class method to bind (in the format '&class::method').
         */
        template<class ClassType, class ParameterType>
        void bindValue(ClassType* classInstance, ParameterType (ClassType::*valueMethod)() const);

        /**
         * Binds the return value of a class method to this material parameter.
         *
         * This overloads the setBinding method to provide support for array parameters.
         * The valueMethod parameter should return an array (pointer) of a supported
         * material parameter type, such as Matrix* for an array of matrices. The
         * countMethod should point to a method that returns the number of entries in
         * the value returned from valueMethod.
         *
         * @param classInstance The instance of the class containing the member method to bind.
         * @param valueMethod A pointer to the class method to bind (in the format '&class::method').
         * @param countMethod A pointer to a method that returns the number of entries in the array returned by valueMethod.
         */
        template<class ClassType, class ParameterType>
        void bindValue(ClassType* classInstance, ParameterType (ClassType::*valueMethod)() const, unsigned int (ClassType::*countMethod)() const);

    private:

        MaterialParameter& operator=(const MaterialParameter&) = delete;


        /**
         * Interface implemented by templated method bindings for simple storage and iteration.
         */
        class MethodBinding
        {
            friend class RenderState;

        public:

            virtual void setValue(const std::shared_ptr<ShaderProgram>& shaderProgram) = 0;

        protected:

            /**
             * Constructor.
             */
            MethodBinding(MaterialParameter* param);


            /**
             * Destructor.
             */
            virtual ~MethodBinding()
            {
            }


            /**
             * Hidden copy assignment operator.
             */
            MethodBinding& operator=(const MethodBinding&) = delete;

            MaterialParameter* _parameter;
            bool _autoBinding;
        };


        /**
         * Defines a method parameter binding for a single value.
         */
        template<class ClassType, class ParameterType>
        class MethodValueBinding : public MethodBinding
        {
            typedef ParameterType (ClassType::*ValueMethod)() const;
        public:
            MethodValueBinding(MaterialParameter* param, ClassType* instance, ValueMethod valueMethod);
            void setValue(const std::shared_ptr<ShaderProgram>& shaderProgram) override;
        private:
            ClassType* _instance;
            ValueMethod _valueMethod;
        };


        /**
         * Defines a method parameter binding for an array of values.
         */
        template<class ClassType, class ParameterType>
        class MethodArrayBinding : public MethodBinding
        {
            typedef ParameterType (ClassType::*ValueMethod)() const;
            typedef size_t (ClassType::*CountMethod)() const;
        public:
            MethodArrayBinding(MaterialParameter* param, ClassType* instance, ValueMethod valueMethod, CountMethod countMethod);
            void setValue(const std::shared_ptr<ShaderProgram>& shaderProgram) override;
        private:
            ClassType* _instance;
            ValueMethod _valueMethod;
            CountMethod _countMethod;
        };


        void clearValue();

        void bind(const std::shared_ptr<ShaderProgram>& shaderProgram);


        enum LOGGER_DIRTYBITS
        {
            UNIFORM_NOT_FOUND = 0x01,
            PARAMETER_VALUE_NOT_SET = 0x02
        };

        boost::variant<std::nullptr_t, float, int, float*, int*, std::shared_ptr<Texture::Sampler>, std::vector<std::shared_ptr<Texture::Sampler>>, std::shared_ptr<MethodBinding>> _value;

        enum
        {
            NONE,
            FLOAT,
            FLOAT_ARRAY,
            INT,
            INT_ARRAY,
            VECTOR2,
            VECTOR3,
            VECTOR4,
            MATRIX,
            SAMPLER,
            SAMPLER_ARRAY,
            METHOD
        } _type;


        size_t _count;
        bool _dynamic;
        std::string _name;
        std::shared_ptr<Uniform> _uniform;
        char _loggerDirtyBits;
    };


    template<class ClassType, class ParameterType>
    void MaterialParameter::bindValue(ClassType* classInstance, ParameterType (ClassType::*valueMethod)() const)
    {
        clearValue();

        _value = std::make_shared<MethodValueBinding<ClassType, ParameterType>>(this, classInstance, valueMethod);
        _dynamic = true;
        _type = MaterialParameter::METHOD;
    }


    template<class ClassType, class ParameterType>
    void MaterialParameter::bindValue(ClassType* classInstance, ParameterType (ClassType::*valueMethod)() const, unsigned int (ClassType::*countMethod)() const)
    {
        clearValue();

        _value = std::make_shared<MethodArrayBinding<ClassType, ParameterType>>(this, classInstance, valueMethod, countMethod);
        _dynamic = true;
        _type = MaterialParameter::METHOD;
    }


    template<class ClassType, class ParameterType>
    MaterialParameter::MethodValueBinding<ClassType, ParameterType>::MethodValueBinding(MaterialParameter* param, ClassType* instance, ValueMethod valueMethod)
        : MethodBinding(param)
        , _instance(instance)
        , _valueMethod(valueMethod)
    {
    }


    template<class ClassType, class ParameterType>
    void MaterialParameter::MethodValueBinding<ClassType, ParameterType>::setValue(const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        shaderProgram->setValue(*_parameter->_uniform, (_instance ->* _valueMethod)());
    }


    template<class ClassType, class ParameterType>
    MaterialParameter::MethodArrayBinding<ClassType, ParameterType>::MethodArrayBinding(MaterialParameter* param, ClassType* instance, ValueMethod valueMethod, CountMethod countMethod)
        : MethodBinding(param)
        , _instance(instance)
        , _valueMethod(valueMethod)
        , _countMethod(countMethod)
    {
    }


    template<class ClassType, class ParameterType>
    void MaterialParameter::MethodArrayBinding<ClassType, ParameterType>::setValue(const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        shaderProgram->setValue(*_parameter->_uniform, (_instance ->* _valueMethod)());
    }
}
