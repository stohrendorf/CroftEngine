#pragma once

#include <memory>
#include <gsl/gsl>

template<typename T, typename... Args>
gsl::not_null<std::shared_ptr<T>> make_not_null_shared(Args&& ... args)
{
    return gsl::make_not_null( std::make_shared<T>( std::forward<Args>( args )... ) );
}
