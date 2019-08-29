#pragma once

#include <chrono>
#include <type_traits>

namespace safe_types
{
    namespace internal
    {
        constexpr intmax_t sign(intmax_t value)
        {
            return (value < 0) ? -1 : 1;
        }

        constexpr intmax_t abs(intmax_t value)
        {
            return value * sign(value);
        }

        constexpr intmax_t gcd(intmax_t first, intmax_t second)
        {
            if (first == 0) {
                return second;
            }
            if (second == 0) {
                return first;
            }
            if (first == second) {
                return first;
            }
            if (first == 1 || second == 1) {
                return 1;
            }
            if (first % 2 == 0 && second % 2 == 0) {
                return 2 * gcd(first / 2, second / 2);
            }
            if (first % 2 == 1 && second % 2 == 0) {
                return gcd(first, second / 2);
            }
            if (first % 2 == 0 && second % 2 == 1) {
                return gcd(first / 2, second);
            }
            if (first % 2 == 1 && second % 2 == 1) {
                return first > second ? gcd(first - second, second) : gcd(first, second - first);
            }
        }

        constexpr intmax_t lcm(intmax_t first, intmax_t second)
        {
            return abs(first * second) / gcd(first, second);
        }

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

    }
    
    
    template<class ToUT,
        typename RatioFrom,
        typename RatioTo,
        class UT>
        constexpr std::enable_if_t<std::is_convertible_v<UT, intmax_t>, ToUT> cast_value(UT&& value)
    {
        using trans_coef = std::ratio_divide<RatioFrom, RatioTo>;
        using common_und_type = std::common_type_t<ToUT, UT, intmax_t>;

        return (
            trans_coef::num == 1 && trans_coef::den == 1
            ? static_cast<ToUT>(value)
            : trans_coef::num != 1 && trans_coef::den == 1
            ? static_cast<ToUT>(static_cast<common_und_type>(value) * static_cast<common_und_type>(trans_coef::num))
            : trans_coef::num == 1 && trans_coef::den != 1
            ? static_cast<ToUT>(static_cast<common_und_type>(value) / static_cast<common_und_type>(trans_coef::den))
            : static_cast<ToUT>(static_cast<common_und_type>(value) * static_cast<common_und_type>(trans_coef::num) / static_cast<common_und_type>(trans_coef::den))
            );
    }

    template<class ToUT,
        typename RatioFrom,
        typename RatioTo,
        class UT>
        constexpr std::enable_if_t<!std::is_convertible_v<UT, intmax_t>, ToUT> cast_value(UT&& value)
    {
        return static_cast<ToUT>(std::move(value));
    }

    template<typename UnderlyingType, typename Period, typename NumInDim, typename DenInDim = internal::tuple_dim<>>
    class complex_type
    {
    public:
        using period = Period;
        using underlying_type = UnderlyingType;
        using dimension_num = NumInDim;
        using dimension_den = DenInDim;

        constexpr UnderlyingType value() const noexcept
        {
            return m_value;
        }

        explicit constexpr complex_type(const UnderlyingType& value)
            : m_value{ value }
        {
        }

        explicit constexpr complex_type(UnderlyingType&& value)
            : m_value{ std::move(value) }
        {
        }

        template<class UT,
            class Ratio,
            typename NumDim,
            typename DenDim>
            constexpr complex_type(const complex_type<UT, Ratio, NumDim, DenDim>& other)
            : m_value{ cast_value<underlying_type, Ratio, period>(other.value()) }
        {
            using div_dim_type = internal::trim<typename internal::join<dimension_num, DenDim>::type, typename internal::join<dimension_den, NumDim>::type>;
            static_assert(internal::is_degenerated<typename div_dim_type::num, typename div_dim_type::den>::value, "Types are not the same dimensions. Operation is vorbidden");
        }

        constexpr complex_type(const complex_type& other)
            : m_value{ cast_value<underlying_type, complex_type::period, period>(other.m_value) }
        {
            using other_complex = std::decay_t<decltype(other)>;
            using other_ratio = other_complex::period;
            m_value = cast_value<underlying_type, other_ratio, period>(other.m_value);
        }

        constexpr complex_type& operator=(const complex_type& other)
        {
            m_value = other.m_value;
        }

        constexpr complex_type(complex_type&& other)
            : m_value{ std::move(other.m_value) }
        {
        }

        constexpr complex_type& operator=(complex_type&& other)
        {
            m_value = std::move(other.m_value);
        }

        constexpr complex_type operator+() const
        {
            return complex_type{ value() };
        }

        constexpr complex_type operator-() const
        {
            return complex_type{ -value() };
        }

        constexpr complex_type& operator++()
        {
            ++m_value;
            return (*this);
        }

        constexpr complex_type operator++(int)
        {
            return (complex_type(m_value++));
        }

        constexpr complex_type& operator--()
        {
            --m_value;
            return (*this);
        }

        constexpr complex_type operator--(int)
        {
            return (complex_type(m_value--));
        }

        constexpr complex_type& operator+=(const complex_type& right)
        {
            m_value += right.m_value;
            return (*this);
        }

        constexpr complex_type& operator-=(const complex_type& right)
        {
            m_value -= right.m_value;
            return (*this);
        }

        constexpr complex_type& operator*=(const underlying_type& right)
        {
            m_value *= right;
            return (*this);
        }

        constexpr complex_type& operator/=(const underlying_type& right)
        {
            m_value /= right;
            return (*this);
        }

        constexpr complex_type& operator%=(const underlying_type& right)
        {
            m_value %= right;
            return (*this);
        }

        constexpr complex_type& operator%=(const complex_type& right)
        {
            m_value %= right.count();
            return (*this);
        }

        template<typename Stream>
        friend Stream& operator <<(Stream& stream, const complex_type& ct)
        {
            return stream << ct.value();
        }

    private:
        UnderlyingType m_value;
    };

    namespace internal
    {
        template<typename T>
        struct _is_complex_type : std::false_type
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
        struct is_degenerated<tuple_dim<T1, Ts1...>, tuple_dim<T2, Ts2...>> :
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
    }

    template<typename UnderlyingType, typename Period, typename DimType>
    using simple_type = complex_type<UnderlyingType, Period, internal::tuple_dim<DimType>>;

    template<typename UnderlyingType, typename DimType>
    using singleton = simple_type<UnderlyingType, std::ratio<1>, internal::tuple_dim<DimType>>;

    template<typename Ratio1, typename Ratio2>
    using common_ratio = std::ratio<internal::gcd(Ratio1::num, Ratio2::num), internal::lcm(Ratio1::den, Ratio2::den)>;

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
        using type = safe_types::complex_type<common_type_t<Und1, Und2>, safe_types::common_ratio<Ratio1, Ratio2>, NumDim1, DenDim1>;
    };

    template<typename UnderlyingType, typename Ratio, typename NumDim, typename DenDim>
    struct hash< safe_types::complex_type<UnderlyingType, Ratio, NumDim, DenDim> >
    {
        size_t operator() (const safe_types::complex_type<UnderlyingType, Ratio, NumDim, DenDim>& ct) const noexcept
        {
            return std::hash<UnderlyingType>{}(ct.value());
        }
    };
}

namespace safe_types
{
    template<class To,
        class UT,
        class Period,
        typename NumDim1,
        typename DenDim1>
        constexpr std::enable_if_t<std::is_convertible_v<UT, intmax_t>, To> cast(const complex_type<UT, Period, NumDim1, DenDim1>& ct)
    {
        using to_und_type = typename To::underlying_type;
        using to_period = typename To::period;
        return static_cast<To>(cast_value<to_und_type, Period, to_period>(ct.value()));
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
        using div_dim_type = internal::trim<typename internal::join<NumDim1, DenDim2>::type, typename internal::join<DenDim1, NumDim2>::type>;
        static_assert(internal::is_degenerated<typename div_dim_type::num, typename div_dim_type::den>::value, "Types are not the same dimensions. Operations +, -, <, == etc are not allowed");
        using common_ut = std::common_type_t<FirstUnderlyingType, SecondUnderlyingType>;
        using common_ratio = safe_types::common_ratio<Ratio1, Ratio2>;
        return cast_value<common_ut, Ratio1, common_ratio>(first.value()) == cast_value<common_ut, Ratio2, common_ratio>(second.value());
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
        using div_dim_type = internal::trim<typename internal::join<NumDim1, DenDim2>::type, typename internal::join<DenDim1, NumDim2>::type>;
        static_assert(internal::is_degenerated<typename div_dim_type::num, typename div_dim_type::den>::value, "Types are not the same dimensions. Operations +, -, <, == etc are not allowed");
        using common_ut = std::common_type_t<FirstUnderlyingType, SecondUnderlyingType>;
        using common_ratio = safe_types::common_ratio<Ratio1, Ratio2>;
        return cast_value<common_ut, Ratio1, common_ratio>(first.value()) < cast_value<common_ut, Ratio2, common_ratio>(second.value());
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
        using div_dim_type = internal::trim<typename internal::join<NumDim1, DenDim2>::type, typename internal::join<DenDim1, NumDim2>::type>;
        static_assert(internal::is_degenerated<typename div_dim_type::num, typename div_dim_type::den>::value, "Types are not the same dimensions. Operations +, -, <, == etc are not allowed");
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
        using common_dim_type = internal::trim<typename internal::join<NumDim1, NumDim2>::type, typename internal::join<DenDim1, DenDim2>::type>;
        constexpr auto gcd12 = internal::gcd(Ratio1::num, Ratio2::den);
        constexpr auto gcd21 = internal::gcd(Ratio2::num, Ratio1::den);
        using common_ratio = std::ratio<
            Ratio1::num / gcd12 * Ratio2::num / gcd21,
            Ratio1::den / gcd21 * Ratio2::den / gcd12 >;
        using common_underlying = std::common_type_t<FirstUnderlyingType, SecondUnderlyingType>;
        using common = complex_type<common_underlying, common_ratio, typename common_dim_type::num, typename common_dim_type::den>;
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
        using common_dim_type = internal::trim<typename internal::join<NumDim1, DenDim2>::type, typename internal::join<DenDim1, NumDim2>::type>;
        constexpr auto gcd_num = internal::gcd(Ratio1::num, Ratio2::num);
        constexpr auto gcd_den = internal::gcd(Ratio2::den, Ratio1::den);
        using common_ratio = std::ratio<
            Ratio1::num / gcd_num * Ratio2::den / gcd_den,
            Ratio1::den / gcd_den * Ratio2::num / gcd_num >;
        using common_underlying = std::common_type_t<FirstUnderlyingType, SecondUnderlyingType>;
        using common = std::conditional_t <
            internal::is_degenerated<typename common_dim_type::num, typename common_dim_type::den>::value,
            common_underlying,
            complex_type<common_underlying, common_ratio, typename common_dim_type::num, typename common_dim_type::den>>;
        return common{ first.value() / second.value() };
    }

    template<typename UnderlyingType,
        typename Ratio1,
        typename NumDim1,
        typename DenDim1,
        typename T>
        constexpr auto operator%(const complex_type<UnderlyingType, Ratio1, NumDim1, DenDim1>& first, const T& val) noexcept
    {
        using type = complex_type<std::common_type_t<UnderlyingType, T>, Ratio1, NumDim1, DenDim1>;
        return type{ first.value() % val };
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename NumDim1,
        typename DenDim1,
        typename NumDim2,
        typename DenDim2>
        constexpr auto operator%(const complex_type<FirstUnderlyingType, Ratio1, NumDim1, DenDim1>& first, const complex_type<SecondUnderlyingType, Ratio2, NumDim2, DenDim2>& second) noexcept
    {
        using div_dim_type = internal::trim<typename internal::join<NumDim1, DenDim2>::type, typename internal::join<DenDim1, NumDim2>::type>;
        static_assert(internal::is_degenerated<typename div_dim_type::num, typename div_dim_type::den>::value, "Types are not the same dimensions. Operation % is not allowed");
        using _CT = std::common_type_t<complex_type<FirstUnderlyingType, Ratio1, NumDim1, DenDim1>, complex_type<SecondUnderlyingType, Ratio2, NumDim2, DenDim2>>;
        return _CT(cast<_CT>(first).value() % cast<_CT>(second).value());
    }

}

