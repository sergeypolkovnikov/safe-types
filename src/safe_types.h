#pragma once

#include <chrono>
#include <type_traits>

namespace safe_types
{
    template<intmax_t _Pn>
    struct _sign
        : std::integral_constant<intmax_t, (_Pn < 0) ? -1 : 1>
    { };

    template<intmax_t _Pn>
    struct _abs
        : std::integral_constant<intmax_t, _Pn * _sign<_Pn>::value>
    { };

    template<intmax_t _Pn, intmax_t _Qn>
    struct _gcd
        : _gcd<_Qn, (_Pn % _Qn)>
    { };

    template<intmax_t _Pn>
    struct _gcd<_Pn, 0>
        : std::integral_constant<intmax_t, _abs<_Pn>::value>
    { };

    template<intmax_t _Qn>
    struct _gcd<0, _Qn>
        : std::integral_constant<intmax_t, _abs<_Qn>::value>
    { };

    template<intmax_t _Pn, intmax_t _Qn>
    struct _lcm
        : std::integral_constant<intmax_t, _abs<_Pn * _Qn>::value / _gcd<_Pn, _Qn>::value>
    { };

    template<typename... Dims>
    struct tuple_dim
    {};

    template <typename T1, typename T2>
    struct join;

    template<typename ...Ts1, typename ...Ts2>
    struct join<tuple_dim<Ts1...>, tuple_dim<Ts2...>>
    {
        using type = tuple_dim<Ts1..., Ts2...>;
    };

    template <typename T, typename Tuple, typename Res = tuple_dim<>>
    struct remove;

    template<typename T, typename Res>
    struct remove<T, tuple_dim<>, Res>
    {
        using type = Res;
    };

    template<typename T, typename... Ts, typename... TRes>
    struct remove<T, tuple_dim<T, Ts...>, tuple_dim<TRes...>>
    {
        using type = tuple_dim<TRes..., Ts...>;
    };

    template<typename T, typename T1, typename ...Ts, typename... TRes>
    struct remove<T, tuple_dim<T1, Ts...>, tuple_dim<TRes...>> :
        remove<T, tuple_dim<Ts...>, tuple_dim<TRes..., T1>>
    {};

    template<typename Tuple1, typename Tuple2>
    struct remove_all {};

    template<typename ...Ts1>
    struct remove_all<tuple_dim<Ts1...>, tuple_dim<>>
    {
        using type = tuple_dim<Ts1...>;
    };

    template<typename ...Ts1, typename T, typename... Ts2>
    struct remove_all<tuple_dim<Ts1...>, tuple_dim<T, Ts2...>>
        : remove_all<typename remove<T, tuple_dim<Ts1...>>::type, tuple_dim<Ts2...>>
    {};

    template<typename T1, typename T2>
    struct trim
    {};

    template<typename ...Ts1, typename ...Ts2>
    struct trim<tuple_dim<Ts1...>, tuple_dim<Ts2...>>
    {
        using num = typename remove_all<tuple_dim<Ts1...>, tuple_dim<Ts2...>>::type;
        using den = typename remove_all<tuple_dim<Ts2...>, tuple_dim<Ts1...>>::type;
    };

    template<typename UnderlyingType>
    class value_type
    {
    public:
        constexpr explicit value_type(const UnderlyingType val) noexcept : m_value{ val }
        {
        }

        constexpr UnderlyingType value() const noexcept
        {
            return m_value;
        }

    private:
        UnderlyingType m_value;
    };

    template<typename UnderlyingType, typename Period, typename NumInDim, typename DenInDim = tuple_dim<>>
    class complex_type : public value_type<UnderlyingType>
    {
    public:
        using period = Period;
        using underlying_type = UnderlyingType;
        using dimension_num = NumInDim;
        using dimension_den = DenInDim;

        explicit constexpr complex_type(const UnderlyingType value)
            : value_type<UnderlyingType>{ value }
        {

        }

        constexpr complex_type operator-() const
        {
            return complex_type{ -value() };
        }
        


        template<typename Stream>
        friend Stream& operator <<(Stream& stream, const complex_type& ct)
        {
            return stream << ct.value();
        }
    };

    template<typename T>
    struct _is_complex_type: std::false_type
    {};

    template<typename UT, typename Ratio, typename Num, typename Den>
    struct _is_complex_type<complex_type<UT, Ratio, Num, Den>> : std::true_type
    {};

    template<typename CT, typename T>
    using _enable_if_is_complex = typename std::enable_if<_is_complex_type<CT>::value, T>::type;

    template<typename T1, typename T2>
    struct is_degenerated
    {};

    template<>
    struct is_degenerated<tuple_dim<>, tuple_dim<>> :
        std::true_type
    {};

    template<typename T1, typename T2, typename...Ts1, typename... Ts2>
    struct is_degenerated<tuple_dim<T1, Ts1...>, tuple_dim<T2, Ts2...>>:
        std::false_type
    {};

    template<typename T1, typename...Ts1>
    struct is_degenerated<tuple_dim<T1, Ts1...>, tuple_dim<>> :
        std::false_type
    {};

    template<typename T2, typename... Ts2>
    struct is_degenerated<tuple_dim<>, tuple_dim<T2, Ts2...>> :
        std::false_type
    {};

    template<typename UnderlyingType, typename Period, typename DimType>
    using simple_type = complex_type<UnderlyingType, Period, tuple_dim<DimType>>;

    template<typename UnderlyingType, typename DimType>
    using singleton = simple_type<UnderlyingType, std::ratio<1>, tuple_dim<DimType>>;

}

namespace std
{
    template<typename Und1,
        typename Ratio1,
        typename Und2,
        typename Ratio2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
        struct common_type<
        safe_types::complex_type<Und1, Ratio1, NumDim1, DenDim1>,
        safe_types::complex_type<Und2, Ratio2, NumDim2, DenDim2>>
    {
        using type = safe_types::complex_type<common_type_t<Und1, Und2>,
            ratio<safe_types::_gcd<Ratio1::num, Ratio2::num>::value,
            safe_types::_lcm<Ratio1::den, Ratio2::den>::value>, NumDim1, DenDim1>;
    };
}

namespace safe_types
{
    template<class To,
        class UT,
        class Period,
        typename NumDim1,
        typename DenDim1>
        constexpr To cast(const complex_type<UT, Period, NumDim1, DenDim1>& ct)
    {
        using trans_coef = std::ratio_divide<Period, typename To::period>;

        using to_und_type = typename To::underlying_type;
        using common_und_type = std::common_type_t<to_und_type, UT, intmax_t>;

        return (trans_coef::num == 1 && trans_coef::den == 1
            ? static_cast<To>(static_cast<to_und_type>(ct.value()))
            : trans_coef::num != 1 && trans_coef::den == 1
            ? static_cast<To>(static_cast<to_und_type>(
                static_cast<common_und_type>(
                    ct.value()) * static_cast<common_und_type>(trans_coef::num)))
            : trans_coef::num == 1 && trans_coef::den != 1
            ? static_cast<To>(static_cast<to_und_type>(
                static_cast<common_und_type>(ct.value())
                / static_cast<common_und_type>(trans_coef::den)))
            : static_cast<To>(static_cast<to_und_type>(
                static_cast<common_und_type>(ct.value()) * static_cast<common_und_type>(trans_coef::num)
                / static_cast<common_und_type>(trans_coef::den))));
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
    constexpr bool
        operator==(const complex_type<FirstUnderlyingType, Ratio1, NumDim1, DenDim1>& first, const complex_type<SecondUnderlyingType, Ratio2, NumDim2, DenDim2>& second) noexcept
    {
        using div_dim_type = trim<join<NumDim1, DenDim2>::type, join<DenDim1, NumDim2>::type>;
        static_assert(is_degenerated<div_dim_type::num, div_dim_type::den>::value, "Types are not the same dimensions. Operations +, -, <, == etc are not allowed");
        using common_type = std::common_type_t<complex_type<FirstUnderlyingType, Ratio1, NumDim1, DenDim1>, complex_type<SecondUnderlyingType, Ratio2, NumDim2, DenDim2>>;
        return cast<common_type>(first).value() == cast<common_type>(second).value();
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
    constexpr bool
        operator!=(const complex_type<FirstUnderlyingType, Ratio1, NumDim1, DenDim1>& first, const complex_type<SecondUnderlyingType, Ratio2, NumDim2, DenDim2>& second) noexcept
    {
        return !(first == second);
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
        constexpr bool operator<(const complex_type<FirstUnderlyingType, Ratio1, NumDim1, DenDim1>& first, const complex_type<SecondUnderlyingType, Ratio2, NumDim2, DenDim2>& second) noexcept
    {
        using div_dim_type = trim<join<NumDim1, DenDim2>::type, join<DenDim1, NumDim2>::type>;
        static_assert(is_degenerated<div_dim_type::num, div_dim_type::den>::value, "Types are not the same dimensions. Operations +, -, <, == etc are not allowed");
        using common_type = std::common_type_t<complex_type<FirstUnderlyingType, Ratio1, NumDim1, DenDim1>, complex_type<SecondUnderlyingType, Ratio2, NumDim2, DenDim2>>;
        return cast<common_type>(first).value() < cast<common_type>(second).value();
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
        constexpr bool operator>(const complex_type<FirstUnderlyingType, Ratio1, NumDim1, DenDim1>& first, const complex_type<SecondUnderlyingType, Ratio2, NumDim2, DenDim2>& second) noexcept
    {
        return second < first;
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
        constexpr bool operator<=(const complex_type<FirstUnderlyingType, Ratio1, NumDim1, DenDim1>& first, const complex_type<SecondUnderlyingType, Ratio2, NumDim2, DenDim2>& second) noexcept
    {
        return !(second < first);
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
        constexpr bool operator>=(const complex_type<FirstUnderlyingType, Ratio1, NumDim1, DenDim1>& first, const complex_type<SecondUnderlyingType, Ratio2, NumDim2, DenDim2>& second) noexcept
    {
        return !(first < second);
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
    constexpr auto operator+(const complex_type<FirstUnderlyingType, Ratio1, NumDim1, DenDim1>& first, const complex_type<SecondUnderlyingType, Ratio2, NumDim2, DenDim2>& second) noexcept
    {
        using div_dim_type = trim<join<NumDim1, DenDim2>::type, join<DenDim1, NumDim2>::type>;
        static_assert(is_degenerated<div_dim_type::num, div_dim_type::den>::value, "Types are not the same dimensions. Operations +, -, <, == etc are not allowed");
        using _CT = std::common_type_t<complex_type<FirstUnderlyingType, Ratio1, NumDim1, DenDim1>, complex_type<SecondUnderlyingType, Ratio2, NumDim2, DenDim2>>;
        return _CT(cast<_CT>(first).value() + cast<_CT>(second).value());
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
    constexpr auto operator-(const complex_type<FirstUnderlyingType, Ratio1, NumDim1, DenDim1>& first, const complex_type<SecondUnderlyingType, Ratio2, NumDim2, DenDim2>& second) noexcept
    {
        return first + (-second);
    }
   
    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
        constexpr auto operator*(const complex_type<FirstUnderlyingType, Ratio1, NumDim1, DenDim1>& first, const complex_type<SecondUnderlyingType, Ratio2, NumDim2, DenDim2>& second) noexcept
    {
        using common_dim_type = trim<join<NumDim1, NumDim2>::type, join<DenDim1, DenDim2>::type>;
        using gcd12 = _gcd<Ratio1::num, Ratio2::den>;
        using gcd21 = _gcd<Ratio2::num, Ratio1::den>;
        using common_ratio = std::ratio<
            Ratio1::num / gcd12::value * Ratio2::num / gcd21::value,
            Ratio1::den / gcd21::value * Ratio2::den / gcd12::value >;
        using common_underlying = std::common_type_t<FirstUnderlyingType, SecondUnderlyingType>;
        using common = complex_type<common_underlying, common_ratio, common_dim_type::num, common_dim_type::den>;
        return common{ first.value() * second.value() };
    }

    template<typename UnderlyingType,
        typename Ratio1,
        typename NumDim1,
        typename DenDim1,
        typename T>
        constexpr auto operator/(const complex_type<UnderlyingType, Ratio1, NumDim1, DenDim1>& first, const T& val) noexcept
    {
        using type = complex_type<std::common_type_t<UnderlyingType, T>, Ratio1, NumDim1, DenDim1>;
        return type{ first.value() / val };
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
        constexpr auto operator/(const complex_type<FirstUnderlyingType, Ratio1, NumDim1, DenDim1>& first, const complex_type<SecondUnderlyingType, Ratio2, NumDim2, DenDim2>& second) noexcept
    {
        using common_dim_type = trim<join<NumDim1, DenDim2>::type, join<DenDim1, NumDim2>::type>;
        using gcd_num = _gcd<Ratio1::num, Ratio2::num>;
        using gcd_den = _gcd<Ratio2::den, Ratio1::den>;
        using common_ratio = std::ratio<
            Ratio1::num / gcd_num::value * Ratio2::den / gcd_den::value,
            Ratio1::den / gcd_den::value * Ratio2::num / gcd_num::value >;
        using common_underlying = std::common_type_t<FirstUnderlyingType, SecondUnderlyingType>;
        using common = std::conditional_t <
            is_degenerated<common_dim_type::num, common_dim_type::den>::value,
            common_underlying,
            complex_type<common_underlying, common_ratio, common_dim_type::num, common_dim_type::den>>;
        return common{ first.value() / second.value() };
    }

}

