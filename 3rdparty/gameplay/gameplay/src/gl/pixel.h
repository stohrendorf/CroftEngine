#pragma once

#include "typetraits.h"


namespace gameplay
{
    namespace gl
    {
        template<typename T>
        struct RGBA final
        {
            static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, "Pixel may only have channels of integral types");

            using Type = T;
            using Traits = TypeTraits<T>;

            static constexpr const auto TypeId = Traits::TypeId;
            static constexpr const auto InternalFormat = Traits::RgbaInternalFormat;
            static constexpr const auto Format = Traits::RgbaFormat;


            explicit RGBA()
                : RGBA{0}
            {
            }


            explicit constexpr RGBA(Type value) noexcept
                : r{value}
                , g{value}
                , b{value}
                , a{value}
            {
            }


            constexpr RGBA(Type r_, Type g_, Type b_, Type a_) noexcept
                : r{r_}
                , g{g_}
                , b{b_}
                , a{a_}
            {
            }


            Type r, g, b, a;
        };


        template<typename T>
        constexpr bool operator==(const RGBA<T>& lhs, const RGBA<T>& rhs)
        {
            return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
        }


        template<typename T>
        constexpr bool operator!=(const RGBA<T>& lhs, const RGBA<T>& rhs)
        {
            return !(lhs == rhs);
        }


        using RGBA8 = RGBA<GLubyte>;


        template<typename T>
        struct RGB final
        {
            static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, "Pixel may only have channels of integral types");

            using Type = T;
            using Traits = TypeTraits<T>;

            static constexpr const auto TypeId = Traits::TypeId;
            static constexpr const auto InternalFormat = Traits::RgbInternalFormat;
            static constexpr const auto Format = Traits::RgbFormat;


            explicit RGB()
                : RGB{0}
            {
            }


            explicit constexpr RGB(Type value) noexcept
                : r{value}
                , g{value}
                , b{value}
            {
            }


            constexpr RGB(Type r_, Type g_, Type b_) noexcept
                : r{r_}
                , g{g_}
                , b{b_}
            {
            }


            Type r, g, b;
        };


        template<typename T>
        constexpr bool operator==(const RGB<T>& lhs, const RGB<T>& rhs)
        {
            return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
        }


        template<typename T>
        constexpr bool operator!=(const RGB<T>& lhs, const RGB<T>& rhs)
        {
            return !(lhs == rhs);
        }


        using RGB8 = RGB<GLubyte>;
    }
}
