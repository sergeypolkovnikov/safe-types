#pragma once

#include "safe_types.h"

namespace safe_types
{
    class DistanceDim;
    using micrometers = simple_type<long long, std::micro, DistanceDim>;
    using millimeters = simple_type<long long, std::milli, DistanceDim>;
    using centimeters = simple_type<long long, std::centi, DistanceDim>;
    using decimeters = simple_type<long long, std::deci, DistanceDim>;
    using meters = simple_type<long long, std::ratio<1>, DistanceDim>;
    using kilometers = simple_type<long long, std::kilo, DistanceDim>;
    using inches = simple_type<long long, std::ratio<10000, 393694>, DistanceDim>;
    using feet = simple_type<long long, std::ratio<120000, 393694>, DistanceDim>;
    using yards = simple_type<long long, std::ratio<360000, 393694>, DistanceDim>;
    using miles = simple_type<long long, std::ratio<633600000, 393694>, DistanceDim>;
    using nautical_miles = simple_type<long long, std::ratio<1852>, DistanceDim>;

    class DurationDim;
    using nanoseconds = simple_type<long long, std::nano, DurationDim>;
    using microseconds = simple_type<long long, std::micro, DurationDim>;
    using milliseconds = simple_type<long long, std::milli, DurationDim>;
    using seconds = simple_type<long long, std::ratio<1>, DurationDim>;
    using minutes = simple_type<int, std::ratio<60>, DurationDim>;
    using hours = simple_type<int, std::ratio<3600>, DurationDim>;
    using days = simple_type<int, std::ratio<86400>, DurationDim>;
    using weeks = simple_type<int, std::ratio<604800>, DurationDim>;

    class WeightDim;
    using milligrams = simple_type<long long, std::milli, WeightDim>;
    using grams = simple_type<long long, std::ratio<1>, WeightDim>;
    using kilograms = simple_type<long long, std::kilo, WeightDim>;
    using tonnes = simple_type < long long, std::mega, WeightDim > ;

    class MemoryVolumeDim;
    using bytes = simple_type<long long, std::ratio<1>, MemoryVolumeDim>;
    using kilobytes = simple_type<long long, std::ratio<1024>, MemoryVolumeDim>;
    using megabytes = simple_type<long long, std::ratio<1048576>, MemoryVolumeDim>;
    using gigabytes = simple_type<long long, std::ratio<1073741824>, MemoryVolumeDim>;
    using terabytes = simple_type<long long, std::ratio<1099511627776>, MemoryVolumeDim>;
}
