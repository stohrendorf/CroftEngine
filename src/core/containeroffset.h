#pragma once

#include <type_traits>
#include <vector>

namespace core
{
namespace detail
{
template<typename T, typename... Args>
struct contains;

template<typename T>
struct contains<T>
{
    static constexpr bool value = false;
};

template<typename T, typename... Args>
struct contains<T, T, Args...>
{
    static constexpr bool value = true;
};

template<typename T, typename A, typename... Args>
struct contains<T, A, Args...>
{
    static constexpr bool value = contains<T, Args...>::value;
};

template<typename T, typename... Args>
constexpr bool contains_v = contains<
        std::remove_cv<std::remove_reference_t<T>>,
        std::remove_cv<std::remove_reference_t<Args>>...
>::value;

template<typename A0, typename... Args>
using first_t = A0;
}

template<typename OffsetType, typename... DataTypes>
struct ContainerOffset
{
    static_assert( std::is_integral_v<OffsetType> && !std::is_signed_v<OffsetType>,
                   "Index type must be unsigned integer like" );
    static_assert( sizeof...( DataTypes ) > 0, "Must provide at least one bound type" );

    using offset_type = OffsetType;

    offset_type offset = 0;

    constexpr ContainerOffset() = default;

    constexpr ContainerOffset(offset_type offset)
            : offset{offset}
    {
    }

    template<typename T>
    explicit ContainerOffset(T) = delete;

    template<typename T>
    offset_type index() const
    {
        static_assert( detail::contains_v<T, DataTypes...>, "Can only use declared types for index conversion" );
        if( offset % sizeof( T ) != 0 )
            throw std::runtime_error( "Offset not dividable by element size" );

        return offset / sizeof( T );
    }

    template<typename T>
    constexpr
    std::enable_if_t<detail::contains_v<T, DataTypes...>, T&>
    from(std::vector<T>& v) const
    {
        if( offset % sizeof( T ) != 0 )
            throw std::runtime_error( "Offset not dividable by element size" );

        return v[offset / sizeof( T )];
    }

    template<typename T>
    constexpr
    std::enable_if_t<detail::contains_v<T, DataTypes...>, T&>
    checkedFrom(std::vector<T>& v) const
    {
        if( offset % sizeof( T ) != 0 )
            throw std::runtime_error( "Offset not dividable by element size" );

        return v.at( offset / sizeof( T ) );
    }

    template<typename T>
    constexpr
    std::enable_if_t<detail::contains_v<T, DataTypes...>, const T&>
    from(const std::vector<T>& v) const
    {
        if( offset % sizeof( T ) != 0 )
            throw std::runtime_error( "Offset not dividable by element size" );

        return v[offset / sizeof( T )];
    }

    template<typename T>
    constexpr
    std::enable_if_t<detail::contains_v<T, DataTypes...>, const T&>
    checkedFrom(const std::vector<T>& v) const
    {
        if( offset % sizeof( T ) != 0 )
            throw std::runtime_error( "Offset not dividable by element size" );

        return v.at( offset / sizeof( T ) );
    }
};


template<typename IndexType, typename... DataTypes>
struct ContainerIndex
{
    static_assert( std::is_integral_v<IndexType> && !std::is_signed_v<IndexType>,
                   "Index type must be unsigned integer like" );
    static_assert( sizeof...( DataTypes ) > 0, "Must provide at least one bound type" );

    using index_type = IndexType;

    index_type index = 0;

    constexpr ContainerIndex() = default;

    constexpr ContainerIndex(index_type index) noexcept
            : index{index}
    {
    }

    template<typename T>
    explicit ContainerIndex(T) = delete;

    template<typename T>
    constexpr
    std::enable_if_t<detail::contains_v<T, DataTypes...>, T&>
    from(std::vector<T>& v) const
    {
        return v[index];
    }

    template<typename T>
    constexpr
    std::enable_if_t<detail::contains_v<T, DataTypes...>, T&>
    checkedFrom(std::vector<T>& v) const
    {
        return v[index];
    }

    template<typename T>
    constexpr
    std::enable_if_t<detail::contains_v<T, DataTypes...>, const T&>
    from(const std::vector<T>& v) const
    {
        return v.at( index );
    }

    template<typename T>
    constexpr
    std::enable_if_t<detail::contains_v<T, DataTypes...>, const T&>
    checkedFrom(const std::vector<T>& v) const
    {
        return v.at( index );
    }

    auto& operator+=(index_type delta)
    {
        if( (index > 0) && (delta > std::numeric_limits<index_type>::max() - index) )
            throw std::out_of_range( "Index addition causes overflow" );

        index += delta;
        return *this;
    }

    auto operator+(const ContainerIndex<IndexType, DataTypes...>& delta) const
    {
        if( (index > 0) && (delta.index > std::numeric_limits<index_type>::max() - index) )
            throw std::out_of_range( "Index addition causes overflow" );

        return index + delta.index;
    }

    template<typename T>
    void operator+=(T) = delete;
};

}