#pragma once

#include <chrono>
#include <type_traits>

namespace safe_types
{
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
        using num_type = typename remove_all<tuple_dim<Ts1...>, tuple_dim<Ts2...>>::type;
        using den_type = typename remove_all<tuple_dim<Ts2...>, tuple_dim<Ts1...>>::type;
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

    template<typename UnderlyingType, typename Period, typename DimType>
    using simple_type = complex_type<UnderlyingType, Period, tuple_dim<DimType>>;

    template<typename UnderlyingType, typename DimType>
    using singleton = simple_type<UnderlyingType, std::ratio<1>, tuple_dim<DimType>>;

}

namespace std
{
    template<typename Und1,
        typename Period1,
        typename Und2,
        typename Period2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
        struct common_type<
        safe_types::complex_type<Und1, Period1, NumDim1, DenDim1>,
        safe_types::complex_type<Und2, Period2, NumDim2, DenDim2>>
    {
        using type = safe_types::complex_type<common_type_t<Und1, Und2>,
            ratio<_Gcd<Period1::num, Period2::num>::value,
            _Lcm<Period1::den, Period2::den>::value>, NumDim1, DenDim1>;
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
        using _CF = std::ratio_divide<Period, typename To::period>;

        using _ToUT = typename To::underlying_type;
        using _CR = std::common_type_t<_ToUT, UT, intmax_t>;

        return (_CF::num == 1 && _CF::den == 1
            ? static_cast<To>(static_cast<_ToUT>(ct.value()))
            : _CF::num != 1 && _CF::den == 1
            ? static_cast<To>(static_cast<_ToUT>(
                static_cast<_CR>(
                    ct.value()) * static_cast<_CR>(_CF::num)))
            : _CF::num == 1 && _CF::den != 1
            ? static_cast<To>(static_cast<_ToUT>(
                static_cast<_CR>(ct.value())
                / static_cast<_CR>(_CF::den)))
            : static_cast<To>(static_cast<_ToUT>(
                static_cast<_CR>(ct.value()) * static_cast<_CR>(_CF::num)
                / static_cast<_CR>(_CF::den))));
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Period1,
        typename Period2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
    constexpr bool
        operator==(const complex_type<FirstUnderlyingType, Period1, NumDim1, DenDim1>& first, const complex_type<SecondUnderlyingType, Period2, NumDim2, DenDim2>& second) noexcept
    {
        using _CT = std::common_type_t<complex_type<FirstUnderlyingType, Period1, NumDim1, DenDim1>, complex_type<SecondUnderlyingType, Period2, NumDim2, DenDim2>>;
        return cast<_CT>(first).value() == cast<_CT>(second).value();
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Period1,
        typename Period2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
    constexpr bool
        operator!=(const complex_type<FirstUnderlyingType, Period1, NumDim1, DenDim1>& first, const complex_type<SecondUnderlyingType, Period2, NumDim2, DenDim2>& second) noexcept
    {
        return !(first == second);
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Period1,
        typename Period2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
    constexpr //std::common_type_t<complex_type<FirstUnderlyingType, Period1, NumDim1, DenDim1>, complex_type<SecondUnderlyingType, Period2, NumDim2, DenDim2>>
        auto operator+(const complex_type<FirstUnderlyingType, Period1, NumDim1, DenDim1>& first, const complex_type<SecondUnderlyingType, Period2, NumDim2, DenDim2>& second) noexcept
    {
        using _CT = std::common_type_t<complex_type<FirstUnderlyingType, Period1, NumDim1, DenDim1>, complex_type<SecondUnderlyingType, Period2, NumDim2, DenDim2>>;
        return _CT(cast<_CT>(first).value() + cast<_CT>(second).value());
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Period1,
        typename Period2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
    constexpr auto //std::common_type_t<complex_type<FirstUnderlyingType, Period1, NumDim1, DenDim1>, complex_type<SecondUnderlyingType, Period2, NumDim2, DenDim2>>
        operator-(const complex_type<FirstUnderlyingType, Period1, NumDim1, DenDim1>& first, const complex_type<SecondUnderlyingType, Period2, NumDim2, DenDim2>& second) noexcept
    {
        return first + (-second);
    }
   
}

