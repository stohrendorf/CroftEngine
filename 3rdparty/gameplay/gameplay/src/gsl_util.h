#pragma once

#include <memory>
#include <gsl/gsl>

template<typename T>
gsl::not_null<std::remove_reference_t<T>> to_not_null(T&& value)
{
    return gsl::not_null<std::remove_reference_t<T>>( std::forward<T>( value ) );
}

template<typename T>
gsl::not_null<std::remove_reference_t<T>> to_not_null(const T& value)
{
    return gsl::not_null<std::remove_reference_t<T>>( value );
}

template<typename T>
gsl::not_null<std::shared_ptr<T>> to_not_null(std::shared_ptr<T>&& value)
{
    return gsl::not_null<std::shared_ptr<T>>( std::move( value ) );
}

template<typename T>
gsl::not_null<std::shared_ptr<T>> to_not_null(const std::shared_ptr<T>& value)
{
    return gsl::not_null<std::shared_ptr<T>>( value );
}

template<typename T, typename... Args>
gsl::not_null<std::shared_ptr<T>> make_not_null_shared(Args&& ... args)
{
    return to_not_null( std::make_shared<T>( std::forward<Args>( args )... ) );
}
