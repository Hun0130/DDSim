/**
 * @file Time_t.cpp
 */
#include <fastdds/rtps/common/Time_t.hpp>

#include <cstdlib>
#include <chrono>

#include <utils/time_t_helpers.hpp>

// 명시적으로 가시성 속성 정의
#ifdef _WIN32
    #define TIME_T_EXPORT __declspec(dllexport)
#else
    #define TIME_T_EXPORT __attribute__((visibility("default")))
#endif

namespace eprosima {
namespace fastdds {
namespace rtps {

TIME_T_EXPORT Time_t::Time_t(
        int32_t sec,
        uint32_t frac)
{
    seconds_ = sec;
    set_fraction(frac);
}

TIME_T_EXPORT Time_t::Time_t(
        long double sec)
{
    seconds_ = static_cast<int32_t>(sec);
    set_fraction(static_cast<uint32_t>((sec - seconds_) * C_FRACTIONS_PER_SEC));
}

TIME_T_EXPORT Time_t::Time_t(
        const eprosima::fastdds::dds::Time_t& time)
{
    seconds_ = time.seconds;
    set_nanosec(time.nanosec);
}

TIME_T_EXPORT int64_t Time_t::to_ns() const
{
    // handle special cases
    // - infinite
    if ( *this == c_RTPSTimeInfinite )
    {
        return -1;
    }
    // - invalid value
    else if ( *this == c_RTPSTimeInvalid )
    {
        return -2;
    }

    int64_t nano = seconds_ * static_cast<int64_t>(C_NANOSECONDS_PER_SEC);
    nano += nanosec_;
    return nano;
}

TIME_T_EXPORT void Time_t::from_ns(
        int64_t nanosecs)
{
    // handle special cases
    // - infinite
    if ( nanosecs == -1 )
    {
        *this = c_RTPSTimeInfinite;
    }
    else if ( nanosecs == -2 )
    {
        *this = c_RTPSTimeInvalid;
    }
    else
    {
        auto res = std::lldiv(nanosecs, 1000000000ull);
        seconds(static_cast<int32_t>(res.quot));
        nanosec(static_cast<uint32_t>(res.rem));
    }
}

TIME_T_EXPORT int32_t Time_t::seconds() const
{
    return seconds_;
}

TIME_T_EXPORT int32_t& Time_t::seconds()
{
    return seconds_;
}

TIME_T_EXPORT void Time_t::seconds(
        int32_t sec)
{
    seconds_ = sec;
}

TIME_T_EXPORT uint32_t Time_t::nanosec() const
{
    return nanosec_;
}

TIME_T_EXPORT void Time_t::nanosec(
        uint32_t nanos)
{
    const uint32_t s_to_nano = static_cast<uint32_t>(C_NANOSECONDS_PER_SEC);
    if (nanos >= s_to_nano)
    {
        nanos %= s_to_nano; // Remove the seconds
    }
    set_nanosec(nanos);
}

TIME_T_EXPORT uint32_t Time_t::fraction() const
{
    return fraction_;
}

TIME_T_EXPORT uint32_t& Time_t::fraction()
{
    return fraction_;
}

TIME_T_EXPORT void Time_t::fraction(
        uint32_t frac)
{
    set_fraction(frac);
}

TIME_T_EXPORT dds::Duration_t Time_t::to_duration_t() const
{
    return dds::Duration_t(seconds_, nanosec_);
}

TIME_T_EXPORT void Time_t::from_duration_t(
        const dds::Duration_t& duration)
{
    seconds_ = duration.seconds;
    set_nanosec(duration.nanosec);
}

TIME_T_EXPORT void Time_t::set_fraction(
        uint32_t frac)
{
    fraction_ = frac;
    nanosec_ = (fraction_ == 0xffffffff)
        ? 0xffffffff
        : frac_to_nano(fraction_);
}

TIME_T_EXPORT void Time_t::set_nanosec(
        uint32_t nanos)
{
    nanosec_ = nanos;

    fraction_ = (nanos == 0xffffffff)
        ? 0xffffffff
        : nano_to_frac(nanos);

    if (fraction_ != 0xffffffff)
    {
        uint32_t nano_check = frac_to_nano(fraction_);
        while (nano_check != nanosec_)
        {
            nano_check = frac_to_nano(++fraction_);
        }
    }
}

TIME_T_EXPORT void Time_t::now(Time_t& ret)
{
    // ret.seconds_ = 0;
    // ret.set_nanosec(0);
}

TIME_T_EXPORT void Time_t::increment_time(
        int32_t sec_increment,
        uint32_t nsec_increment)
{
    seconds_ += sec_increment;
    nanosec_ += nsec_increment;
    if (nanosec_ >= C_NANOSECONDS_PER_SEC)
    {
        seconds_ += nanosec_ / C_NANOSECONDS_PER_SEC;
        nanosec_ %= C_NANOSECONDS_PER_SEC;
    }
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima