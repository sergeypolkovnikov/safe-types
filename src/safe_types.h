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

        template<typename T, bool Val = std::is_arithmetic<std::decay_t<T>>::value>
        struct parameter_for_copy
        {
            using type = const T;
        };

        template<typename T>
        struct parameter_for_copy<T, false>
        {
            using type = const T&;
        };

        template<typename T>
        using parameter_for_copy_t = typename parameter_for_copy<T>::type;

        template<typename T>
        using enable_if_not_arithmetic = std::enable_if<std::is_arithmetic<T>::value>;

        template<typename... Dims>
        struct tuple_dim {};

        template<typename Num, typename Den>
        struct dim_ratio {};

        template<typename ... Nums, typename ... Dens>
        struct dim_ratio<tuple_dim<Nums...>, tuple_dim<Dens...>>
        {
            using num = tuple_dim<Nums...>;
            using den = tuple_dim<Dens...>;
        };

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

        template<typename T1>
        struct is_degenerated
        {};

        template<>
        struct is_degenerated<dim_ratio<tuple_dim<>, tuple_dim<>>> :
            std::true_type
        {};

        template<typename T1, typename T2, typename...Ts1, typename... Ts2>
        struct is_degenerated<dim_ratio<tuple_dim<T1, Ts1...>, tuple_dim<T2, Ts2...>>> :
            std::false_type
        {};

        template<typename T1, typename...Ts1>
        struct is_degenerated<dim_ratio<tuple_dim<T1, Ts1...>, tuple_dim<>>> :
            std::false_type
        {};

        template<typename T2, typename... Ts2>
        struct is_degenerated<dim_ratio<tuple_dim<>, tuple_dim<T2, Ts2...>>> :
            std::false_type
        {};

        template<typename T1, typename T2>
        using trim_is_degenerated_v =
            typename safe_types::internal::is_degenerated<
                safe_types::internal::dim_ratio<
                    typename safe_types::internal::trim<T1, T2>::num,
                    typename safe_types::internal::trim<T1, T2>::den
            >>::value;

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

    }

    template<typename T1, typename T2>
    struct is_same {};

    template<typename ...Num1, typename ...Den1, typename ...Num2, typename ...Den2>
    struct is_same<
        safe_types::internal::dim_ratio<
        safe_types::internal::tuple_dim<Num1...>,
        safe_types::internal::tuple_dim<Den1...>>,
        safe_types::internal::dim_ratio<
        safe_types::internal::tuple_dim<Num2...>,
        safe_types::internal::tuple_dim<Den2...>>>
        : std::conditional_t<
        safe_types::internal::is_degenerated<
        safe_types::internal::dim_ratio<
        typename safe_types::internal::trim<
        typename safe_types::internal::join<
        safe_types::internal::tuple_dim<Num1...>,
        safe_types::internal::tuple_dim<Den2...>>::type,
        typename safe_types::internal::join<
        safe_types::internal::tuple_dim<Num2...>,
        safe_types::internal::tuple_dim<Den1...>>::type>::num,
        typename safe_types::internal::trim<
        typename safe_types::internal::join<
        safe_types::internal::tuple_dim<Num1...>,
        safe_types::internal::tuple_dim<Den2...>>::type,
        typename safe_types::internal::join<
        safe_types::internal::tuple_dim<Num2...>,
        safe_types::internal::tuple_dim<Den1...>>::type>::den
        >>::value,
        std::true_type,
        std::false_type>
    {};
}

namespace safe_types
{
    namespace internal
    {
        template<typename Dim1, typename Dim2>
        using DimIsConvertible = std::enable_if_t<is_same<Dim1, Dim2>::value>;
    }

    template<typename UnderlyingType, typename Ratio, typename DimRatio, typename Limitations = limitations<true, true, true>>
    class complex_type {};

    template<bool arithmetic, bool ordering, bool stream>
    struct limitations
    {
        static constexpr bool enableArithmetic = arithmetic;
        static constexpr bool enableOrdering = ordering;
        static constexpr bool enableStream = stream;
    };

    namespace internal
    {
        template<typename Lim1, typename Lim2 = Lim1>
        using arithmetic_enabled = std::enable_if_t<Lim1::enableArithmetic && Lim2::enableArithmetic>;

        template<typename Lim1, typename Lim2 = Lim1>
        using ordering_enabled = std::enable_if_t<Lim1::enableOrdering && Lim2::enableOrdering>;

        template<typename Lim>
        using streaming_enabled = std::enable_if_t<Lim::enableStream>;
    }

    template<typename UnderlyingType, intmax_t Num, intmax_t Den, typename ... DimNums, typename ... DimDens, typename Limitations>
    class complex_type<UnderlyingType, std::ratio<Num, Den>, internal::dim_ratio<internal::tuple_dim<DimNums...>, internal::tuple_dim<DimDens...>>, Limitations>
    {
    public:
        using period = std::ratio<Num, Den>;
        using underlying_type = UnderlyingType;
        using dimensions = internal::dim_ratio<internal::tuple_dim<DimNums...>, internal::tuple_dim<DimDens...>>;
        using limitations = Limitations;

        constexpr UnderlyingType value() const noexcept
        {
            return m_value;
        }

        template<typename = std::enable_if<std::is_default_constructible<typename underlying_type>::value>::type>
        constexpr complex_type()
            : m_value {}
        {
        }

        explicit constexpr complex_type(internal::parameter_for_copy_t<UnderlyingType> value)
            : m_value{ value }
        {
        }

        template<typename = internal::enable_if_not_arithmetic<UnderlyingType>>
        explicit constexpr complex_type(UnderlyingType&& value)
            : m_value{ std::move(value) }
        {
        }

        template<typename UnderlyingType, intmax_t Num, intmax_t Den, typename ... DimNums, typename ... DimDens, 
            typename = internal::DimIsConvertible<internal::dim_ratio<internal::tuple_dim<DimNums...>, internal::tuple_dim<DimDens...>>, dimensions>>
        constexpr complex_type(const complex_type<UnderlyingType, std::ratio<Num, Den>, internal::dim_ratio<internal::tuple_dim<DimNums...>, internal::tuple_dim<DimDens...>>>& other)
            : m_value{ internal::cast_value<underlying_type, std::ratio<Num, Den>, period>(other.value()) }
        {
        }

        template<typename UnderlyingType, intmax_t Num, intmax_t Den, typename ... DimNums, typename ... DimDens,
            typename = internal::DimIsConvertible<internal::dim_ratio<internal::tuple_dim<DimNums...>, internal::tuple_dim<DimDens...>>, dimensions>>
            constexpr complex_type& operator=(const complex_type<UnderlyingType, std::ratio<Num, Den>, internal::dim_ratio<internal::tuple_dim<DimNums...>, internal::tuple_dim<DimDens...>>>& other)
        {
            m_value = internal::cast_value<underlying_type, std::ratio<Num, Den>, period>(other.value());
        }

        constexpr complex_type(const complex_type& other)
            : m_value{other.m_value}
        {
        }

        constexpr complex_type& operator=(const complex_type& other)
        {
            m_value = other.m_value;
            return *this;
        }

        constexpr complex_type(complex_type&& other)
            : m_value{ std::move(other.m_value) }
        {
        }

        constexpr complex_type& operator=(complex_type&& other)
        {
            m_value = std::move(other.m_value);
            return *this;
        }

        constexpr complex_type operator+() const
        {
            return complex_type{ value() };
        }

        constexpr complex_type operator-() const
        {
            return complex_type{ -value() };
        }

        template<typename = internal::arithmetic_enabled<limitations>>
        constexpr complex_type& operator++()
        {
            ++m_value;
            return (*this);
        }

        template<typename = internal::arithmetic_enabled<limitations>>
        constexpr complex_type operator++(int)
        {
            return (complex_type(m_value++));
        }

        template<typename = internal::arithmetic_enabled<limitations>>
        constexpr complex_type& operator--()
        {
            --m_value;
            return (*this);
        }

        template<typename = internal::arithmetic_enabled<limitations>>
        constexpr complex_type operator--(int)
        {
            return (complex_type(m_value--));
        }

        template<typename = internal::arithmetic_enabled<limitations>>
        constexpr complex_type& operator+=(const complex_type& right)
        {
            m_value += right.m_value;
            return (*this);
        }

        template<typename = internal::arithmetic_enabled<limitations>>
        constexpr complex_type& operator-=(const complex_type& right)
        {
            m_value -= right.m_value;
            return (*this);
        }

        template<typename = internal::arithmetic_enabled<limitations>>
        constexpr complex_type& operator*=(internal::parameter_for_copy_t<UnderlyingType> right)
        {
            m_value *= right;
            return (*this);
        }

        template<typename = internal::arithmetic_enabled<limitations>>
        constexpr complex_type& operator/=(internal::parameter_for_copy_t<UnderlyingType> right)
        {
            m_value /= right;
            return (*this);
        }

        template<typename = internal::arithmetic_enabled<limitations>>
        constexpr complex_type& operator%=(internal::parameter_for_copy_t<UnderlyingType> right)
        {
            m_value %= right;
            return (*this);
        }

        template<typename = internal::arithmetic_enabled<limitations>>
        constexpr complex_type& operator%=(const complex_type& right)
        {
            m_value %= right.value();
            return (*this);
        }

        template<typename Stream, typename = internal::streaming_enabled<limitations>>
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

        template<typename UT, typename Ratio, typename DimRatio>
        struct _is_complex_type<complex_type<UT, Ratio, DimRatio>> : std::true_type
        {};

        template<typename CT, typename T>
        using _enable_if_is_complex = typename std::enable_if<_is_complex_type<CT>::value, T>::type;
    }

    template<typename UnderlyingType, typename Ratio, typename DimType>
    using simple_type = complex_type<UnderlyingType, Ratio, internal::dim_ratio<internal::tuple_dim<DimType>, internal::tuple_dim<>>>;

    template<typename UnderlyingType, typename DimType>
    using singleton = simple_type<UnderlyingType, std::ratio<1>, DimType>;

    template<typename Ratio1, typename Ratio2>
    using common_ratio = std::ratio<internal::gcd(Ratio1::num, Ratio2::num), internal::lcm(Ratio1::den, Ratio2::den)>;

}

namespace std
{
    template<typename Und1,
        typename Ratio1,
        typename Und2,
        typename Ratio2,
        typename Dim1,
        typename Dim2>
        struct common_type<
            safe_types::complex_type<Und1, Ratio1, Dim1>,
            safe_types::complex_type<Und2, Ratio2, Dim2>>
    {
        using type = safe_types::complex_type<common_type_t<Und1, Und2>, safe_types::common_ratio<Ratio1, Ratio2>, Dim1>;
    };

    template<typename UnderlyingType, typename Ratio, typename Dim>
    struct hash< safe_types::complex_type<UnderlyingType, Ratio, Dim> >
    {
        size_t operator() (const safe_types::complex_type<UnderlyingType, Ratio, Dim>& ct) const noexcept
        {
            return std::hash<UnderlyingType>{}(ct.value());
        }
    };
}

namespace safe_types
{
    template<typename To,
        typename UT,
        typename Ratio,
        typename Dim>
        constexpr std::enable_if_t<std::is_convertible_v<UT, intmax_t>, To> cast(const complex_type<UT, Ratio, Dim>& ct)
    {
        using to_und_type = typename To::underlying_type;
        using to_period = typename To::period;
        return static_cast<To>(internal::cast_value<to_und_type, Ratio, to_period>(ct.value()));
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename Dim1,
        typename Dim2,
        typename = internal::DimIsConvertible<Dim1, Dim2>>
        constexpr bool
        operator==(const complex_type<FirstUnderlyingType, Ratio1, Dim1>& first, const complex_type<SecondUnderlyingType, Ratio2, Dim2>& second) noexcept
    {
        using common_ut = std::common_type_t<FirstUnderlyingType, SecondUnderlyingType>;
        using common_ratio = safe_types::common_ratio<Ratio1, Ratio2>;
        return internal::cast_value<common_ut, Ratio1, common_ratio>(first.value()) == internal::cast_value<common_ut, Ratio2, common_ratio>(second.value());
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename Dim1,
        typename Dim2,
        typename = internal::DimIsConvertible<Dim1, Dim2>>
        constexpr bool
        operator!=(const complex_type<FirstUnderlyingType, Ratio1, Dim1>& first, const complex_type<SecondUnderlyingType, Ratio2, Dim2>& second) noexcept
    {
        return !(first == second);
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename Dim1,
        typename Dim2,
        typename Lim1,
        typename Lim2,
        typename = internal::ordering_enabled<Lim1, Lim2>,
        typename = internal::DimIsConvertible<Dim1, Dim2>>
        constexpr bool operator<(const complex_type<FirstUnderlyingType, Ratio1, Dim1, Lim1>& first, const complex_type<SecondUnderlyingType, Ratio2, Dim2, Lim2>& second) noexcept
    {
        using common_ut = std::common_type_t<FirstUnderlyingType, SecondUnderlyingType>;
        using common_ratio = safe_types::common_ratio<Ratio1, Ratio2>;
        return internal::cast_value<common_ut, Ratio1, common_ratio>(first.value()) < internal::cast_value<common_ut, Ratio2, common_ratio>(second.value());
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename Dim1,
        typename Dim2,
        typename Lim1,
        typename Lim2,
        typename = internal::ordering_enabled<Lim1, Lim2>,
        typename = safe_types::internal::DimIsConvertible<Dim1, Dim2>>
        constexpr bool operator>(const complex_type<FirstUnderlyingType, Ratio1, Dim1, Lim1>& first, const complex_type<SecondUnderlyingType, Ratio2, Dim2, Lim2>& second) noexcept
    {
        return second < first;
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename Dim1,
        typename Dim2,
        typename Lim1,
        typename Lim2,
        typename = internal::ordering_enabled<Lim1, Lim2>,
        typename = internal::DimIsConvertible<Dim1, Dim2>>
        constexpr bool operator<=(const complex_type<FirstUnderlyingType, Ratio1, Dim1, Lim1>& first, const complex_type<SecondUnderlyingType, Ratio2, Dim2, Lim2>& second) noexcept
    {
        return !(second < first);
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename Dim1,
        typename Dim2,
        typename Lim1,
        typename Lim2,
        typename = internal::ordering_enabled<Lim1, Lim2>,
        typename = internal::DimIsConvertible<Dim1, Dim2>>
        constexpr bool operator>=(const complex_type<FirstUnderlyingType, Ratio1, Dim1, Lim1>& first, const complex_type<SecondUnderlyingType, Ratio2, Dim2, Lim2>& second) noexcept
    {
        return !(first < second);
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename Dim1,
        typename Dim2,
        typename Lim1,
        typename Lim2,
        typename = internal::arithmetic_enabled<Lim1, Lim2>,
        typename = internal::DimIsConvertible<Dim1, Dim2>>
        constexpr auto operator+(const complex_type<FirstUnderlyingType, Ratio1, Dim1, Lim1>& first, const complex_type<SecondUnderlyingType, Ratio2, Dim2, Lim2>& second) noexcept
    {
        using _CT = std::common_type_t<complex_type<FirstUnderlyingType, Ratio1, Dim1>, complex_type<SecondUnderlyingType, Ratio2, Dim2>>;
        return _CT(cast<_CT>(first).value() + cast<_CT>(second).value());
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename Dim1,
        typename Dim2,
        typename Lim1,
        typename Lim2,
        typename = internal::arithmetic_enabled<Lim1, Lim2>,
        typename = internal::DimIsConvertible<Dim1, Dim2>>
        constexpr auto operator-(const complex_type<FirstUnderlyingType, Ratio1, Dim1, Lim1>& first, const complex_type<SecondUnderlyingType, Ratio2, Dim2, Lim2>& second) noexcept
    {
        return first + (-second);
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename Dim1,
        typename Dim2,
        typename Lim1,
        typename Lim2,
        typename = internal::arithmetic_enabled<Lim1, Lim2>>
        constexpr auto operator*(const complex_type<FirstUnderlyingType, Ratio1, Dim1, Lim1>& first, const complex_type<SecondUnderlyingType, Ratio2, Dim2, Lim2>& second) noexcept
    {
        using common_dim_type = internal::trim<typename internal::join<typename Dim1::num, typename Dim2::num>::type, typename internal::join<typename Dim1::den, typename Dim2::den>::type>;
        constexpr auto gcd12 = internal::gcd(Ratio1::num, Ratio2::den);
        constexpr auto gcd21 = internal::gcd(Ratio2::num, Ratio1::den);
        using common_ratio = std::ratio<
            Ratio1::num / gcd12 * Ratio2::num / gcd21,
            Ratio1::den / gcd21 * Ratio2::den / gcd12 >;
        using common_underlying = std::common_type_t<FirstUnderlyingType, SecondUnderlyingType>;
        using common = std::conditional_t <
            internal::is_degenerated<internal::dim_ratio<typename common_dim_type::num, typename common_dim_type::den>>::value,
            common_underlying,
            complex_type<common_underlying, common_ratio, internal::dim_ratio<typename common_dim_type::num, typename common_dim_type::den>>>;
        return common{ first.value() * second.value() };
    }

    template<typename UnderlyingType,
        typename Ratio1,
        typename Dim,
        typename T,
        typename Lim,
        typename = internal::arithmetic_enabled<Lim>>
        constexpr auto operator*(const complex_type<UnderlyingType, Ratio1, Dim, Lim>& first, const T& val) noexcept
    {
        using type = complex_type<std::common_type_t<UnderlyingType, T>, Ratio1, Dim>;
        return type{ first.value() * val };
    }

    template<typename UnderlyingType,
        typename Ratio1,
        typename Dim,
        typename T,
        typename Lim,
        typename = internal::arithmetic_enabled<Lim>>
        constexpr auto operator*(const T& val, const complex_type<UnderlyingType, Ratio1, Dim, Lim>& first) noexcept
    {
        using type = complex_type<std::common_type_t<UnderlyingType, T>, Ratio1, Dim>;
        return type{ first.value() * val };
    }

    template<typename UnderlyingType,
        typename Ratio1,
        typename Dim,
        typename T,
        typename Lim,
        typename = internal::arithmetic_enabled<Lim>>
        constexpr auto operator/(const complex_type<UnderlyingType, Ratio1, Dim, Lim>& first, const T& val) noexcept
    {
        using type = complex_type<std::common_type_t<UnderlyingType, T>, Ratio1, Dim>;
        return type{ first.value() / val };
    }

    template<typename UnderlyingType,
        typename Ratio1,
        typename Dim,
        typename T,
        typename Lim,
        typename = internal::arithmetic_enabled<Lim>>
        constexpr auto operator/(const T& val, const complex_type<UnderlyingType, Ratio1, Dim, Lim>& first) noexcept
    {
        using type = complex_type<std::common_type_t<UnderlyingType, T>, std::ratio<Ratio1::den, Ratio1::num>, internal::dim_ratio<Dim::den, Dim::num>>;
        return type{ val / first.value() };
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename Dim1,
        typename Dim2,
        typename Lim1,
        typename Lim2,
        typename = internal::arithmetic_enabled<Lim1, Lim2>>
        constexpr auto operator/(const complex_type<FirstUnderlyingType, Ratio1, Dim1, Lim1>& first, const complex_type<SecondUnderlyingType, Ratio2, Dim2, Lim2>& second) noexcept
    {
        using common_dim_type = internal::trim<typename internal::join<typename Dim1::num, typename Dim2::den>::type, typename internal::join<typename Dim1::den, typename Dim2::num>::type>;
        constexpr auto gcd_num = internal::gcd(Ratio1::num, Ratio2::num);
        constexpr auto gcd_den = internal::gcd(Ratio2::den, Ratio1::den);
        using common_ratio = std::ratio<
            Ratio1::num / gcd_num * Ratio2::den / gcd_den,
            Ratio1::den / gcd_den * Ratio2::num / gcd_num >;
        using common_underlying = std::common_type_t<FirstUnderlyingType, SecondUnderlyingType>;
        using common = std::conditional_t <
            internal::is_degenerated<internal::dim_ratio<typename common_dim_type::num, typename common_dim_type::den>>::value,
            common_underlying,
            complex_type<common_underlying, common_ratio, internal::dim_ratio<typename common_dim_type::num, typename common_dim_type::den>>>;
        return common{ first.value() / second.value() };
    }

    template<typename UnderlyingType,
        typename Ratio1,
        typename Dim,
        typename T,
        typename Lim,
        typename = internal::arithmetic_enabled<Lim>>
        constexpr auto operator%(const complex_type<UnderlyingType, Ratio1, Dim, Lim>& first, const T& val) noexcept
    {
        using type = complex_type<std::common_type_t<UnderlyingType, T>, Ratio1, Dim>;
        return type{ first.value() % val };
    }

    template<typename FirstUnderlyingType,
        typename SecondUnderlyingType,
        typename Ratio1,
        typename Ratio2,
        typename Dim1,
        typename Dim2,
        typename Lim1,
        typename Lim2,
        typename = internal::arithmetic_enabled<Lim1, Lim2>,
        typename = internal::DimIsConvertible<Dim1, Dim2>>
        constexpr auto operator%(const complex_type<FirstUnderlyingType, Ratio1, Dim1, Lim1>& first, const complex_type<SecondUnderlyingType, Ratio2, Dim2, Lim2>& second) noexcept
    {
        using _CT = std::common_type_t<complex_type<FirstUnderlyingType, Ratio1, Dim1>, complex_type<SecondUnderlyingType, Ratio2, Dim2>>;
        return _CT(cast<_CT>(first).value() % cast<_CT>(second).value());
    }

}

