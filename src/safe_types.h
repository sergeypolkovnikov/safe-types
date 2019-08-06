#pragma once

#include <chrono>
#include <type_traits>

namespace safe_types
{
    template <typename T1, typename T2>
    struct join;

    template<typename ...Ts1, typename ...Ts2>
    struct join<std::tuple<Ts1...>, std::tuple<Ts2...>>
    {
        using type = std::tuple<Ts1..., Ts2...>;
    };

    template <typename T, typename Tuple, typename Res = std::tuple<>>
    struct remove;

    template<typename T, typename Res>
    struct remove<T, std::tuple<>, Res>
    {
        using type = Res;
    };

    template<typename T, typename... Ts, typename... TRes>
    struct remove<T, std::tuple<T, Ts...>, std::tuple<TRes...>>
    {
        using type = std::tuple<TRes..., Ts...>;
    };

    template<typename T, typename T1, typename ...Ts, typename... TRes>
    struct remove<T, std::tuple<T1, Ts...>, std::tuple<TRes...>> :
        remove<T, std::tuple<Ts...>, std::tuple<TRes..., T1>>
    {};

    template<typename Tuple1, typename Tuple2>
    struct remove_all {};

    template<typename ...Ts1>
    struct remove_all<std::tuple<Ts1...>, std::tuple<>>
    {
        using type = std::tuple<Ts1...>;
    };

    template<typename ...Ts1, typename T, typename... Ts2>
    struct remove_all<std::tuple<Ts1...>, std::tuple<T, Ts2...>>
        : remove_all<typename remove<T, std::tuple<Ts1...>>::type, std::tuple<Ts2...>>
    {};

    template<typename T1, typename T2>
    struct trim
    {};

    template<typename ...Ts1, typename ...Ts2>
    struct trim<std::tuple<Ts1...>, std::tuple<Ts2...>>
    {
        using num_type = typename remove_all<std::tuple<Ts1...>, std::tuple<Ts2...>>::type;
        using den_type = typename remove_all<std::tuple<Ts2...>, std::tuple<Ts1...>>::type;
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

    template<typename UnderlyingType, typename Period, size_t NumeratorCount, typename ... Types>
    class complex_type : public value_type<UnderlyingType>
    {
    public:
        using period = Period;
        using underlying_type = UnderlyingType;

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
    using simple_type = complex_type<UnderlyingType, Period, 1, DimType>;

    template<typename UnderlyingType, typename DimType>
    using singleton = simple_type<UnderlyingType, std::ratio<1>, DimType>;

}

namespace std
{
    template<typename Und1,
        typename Period1,
        typename Und2,
        typename Period2,
        size_t NumeratorCount,
        typename ... Types>
        struct common_type<
        safe_types::complex_type<Und1, Period1, NumeratorCount, Types...>,
        safe_types::complex_type<Und2, Period2, NumeratorCount, Types...>>
    {
        using type = safe_types::complex_type<common_type_t<Und1, Und2>,
            ratio<_Gcd<Period1::num, Period2::num>::value,
            _Lcm<Period1::den, Period2::den>::value>, NumeratorCount, Types...>;
    };
}

namespace safe_types
{
    template<class To,
        class UT,
        class Period,
        size_t NumeratorCount,
        typename ... Types>
        constexpr To cast(const complex_type<UT, Period, NumeratorCount, Types...>& ct)
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

    template<typename FirstUnderlyingType, typename SecondUnderlyingType, typename Period1, typename Period2, size_t NumeratorCount, typename ... Types>
    constexpr bool
        operator==(const complex_type<FirstUnderlyingType, Period1, NumeratorCount, Types...>& first, const complex_type<SecondUnderlyingType, Period2, NumeratorCount, Types...>& second) noexcept
    {
        using _CT = std::common_type_t<complex_type<FirstUnderlyingType, Period1, NumeratorCount, Types...>, complex_type<SecondUnderlyingType, Period2, NumeratorCount, Types...>>;
        return cast<_CT>(first).value() == cast<_CT>(second).value();
    }

    template<typename FirstUnderlyingType, typename SecondUnderlyingType, typename Period1, typename Period2, size_t NumeratorCount, typename ... Types>
    constexpr bool
        operator!=(const complex_type<FirstUnderlyingType, Period1, NumeratorCount, Types...>& first, const complex_type<SecondUnderlyingType, Period2, NumeratorCount, Types...>& second) noexcept
    {
        return !(first == second);
    }

    template<typename FirstUnderlyingType, typename SecondUnderlyingType, typename Period1, typename Period2, size_t NumeratorCount, typename ... Types>
    constexpr std::common_type_t<complex_type<FirstUnderlyingType, Period1, NumeratorCount, Types...>, complex_type<SecondUnderlyingType, Period2, NumeratorCount, Types...>>
        operator+(const complex_type<FirstUnderlyingType, Period1, NumeratorCount, Types...>& first, const complex_type<SecondUnderlyingType, Period2, NumeratorCount, Types...>& second) noexcept
    {
        using _CT = std::common_type_t<complex_type<FirstUnderlyingType, Period1, NumeratorCount, Types...>, complex_type<SecondUnderlyingType, Period2, NumeratorCount, Types...>>;
        return _CT(cast<_CT>(first).value() + cast<_CT>(second).value());
    }

    template<typename FirstUnderlyingType, typename SecondUnderlyingType, typename Period1, typename Period2, size_t NumeratorCount, typename ... Types>
    constexpr std::common_type_t<complex_type<FirstUnderlyingType, Period1, NumeratorCount, Types...>, complex_type<SecondUnderlyingType, Period2, NumeratorCount, Types...>>
        operator-(const complex_type<FirstUnderlyingType, Period1, NumeratorCount, Types...>& first, const complex_type<SecondUnderlyingType, Period2, NumeratorCount, Types...>& second) noexcept
    {
        return first + (-second);
    }
   
}

