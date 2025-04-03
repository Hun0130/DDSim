/**
 * @file Time_t.hpp  // 파일 이름 및 역할을 명시
 */

#ifndef FASTDDS_DDS_CORE__TIME_T_HPP  // 헤더 파일 다중 포함 방지를 위한 매크로 시작
#define FASTDDS_DDS_CORE__TIME_T_HPP  // 매크로 정의

#include <fastdds/fastdds_dll.hpp>  // DLL 내보내기/가져오기 관련 매크로 정의 포함

#include <cmath>      // 수학 관련 함수(예: pow 등) 사용을 위한 헤더
#include <cstdint>    // 고정 크기 정수형 (int32_t, uint32_t 등) 사용을 위한 헤더
#include <iostream>   // 입출력 스트림 (std::ostream 등) 사용을 위한 헤더

namespace eprosima {  // eprosima 네임스페이스 시작
namespace fastdds {    // fastdds 네임스페이스 시작
namespace dds {        // dds 네임스페이스 시작

/**
 * DDS 레벨에서 사용되는 시간을 나타내는 구조체 (Time_t)
 */
struct FASTDDS_EXPORTED_API Time_t
{
    // 무한대 시간을 나타내는 초 값 (32비트 정수 최대값)
    static constexpr int32_t INFINITE_SECONDS = 0x7fffffff;
    // 무한대 시간을 나타내는 나노초 값 (32비트 부호 없는 정수 최대값)
    static constexpr uint32_t INFINITE_NANOSECONDS = 0xffffffffu;

    int32_t seconds;    // 초 단위 시간 저장
    uint32_t nanosec;   // 나노초 단위 시간 저장

    //! 기본 생성자: seconds와 nanosec를 0으로 초기화
    Time_t();

    /**
     * @param sec 초 단위 시간
     * @param nsec 나노초 단위 시간
     */
    Time_t(
            int32_t sec,
            uint32_t nsec);

    /**
     * @param sec 소수점이 포함된 초 값. 소수 부분은 나노초로 변환됨.
     */
    Time_t(
            long double sec);

    // 내부 나노초 값을 설정할 때 fraction 값을 설정하는 함수
    void fraction(
            uint32_t frac);

    // 현재 객체의 fraction 값을 반환하는 함수 (실제로는 nanosec 값을 특정 fraction 값으로 변환)
    uint32_t fraction() const;

    /**
     * 저장된 시간(초와 나노초 포함)을 나노초 단위로 반환.
     */
    int64_t to_ns() const;

    // 현재 객체가 무한대 시간을 나타내는지 판단하는 인라인 함수 (예외 없음)
    inline bool is_infinite() const noexcept
    {
        return is_infinite(*this);  // 아래 정의된 정적 함수 is_infinite를 호출
    }

    static void now(Time_t& ret);

    /**
     * 시간을 증가시키는 함수
     * 
     * @param sec_increment 증가시킬 초 단위 시간
     * @param nsec_increment 증가시킬 나노초 단위 시간
     */
    void increment_time(
            int32_t sec_increment,
            uint32_t nsec_increment);

    // 주어진 Time_t 객체가 무한대 시간(INFINITE_SECONDS 또는 INFINITE_NANOSECONDS)을 가지는지 확인하는 정적 인라인 constexpr 함수
    static inline constexpr bool is_infinite(
            const Time_t& t) noexcept
    {
        return (INFINITE_SECONDS == t.seconds) || (INFINITE_NANOSECONDS == t.nanosec);
    }

};

using Duration_t = Time_t;  // Duration_t는 Time_t의 별칭

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * 두 Time_t 객체가 같은지 비교하는 연산자.
 * @param t1 비교할 첫 번째 Time_t 객체
 * @param t2 비교할 두 번째 Time_t 객체
 * @return 두 객체가 동일하면 true, 아니면 false 반환
 */
static inline bool operator ==(
        const Time_t& t1,
        const Time_t& t2)
{
    if (t1.seconds != t2.seconds)
    {
        return false;  // 초 값이 다르면 false
    }
    if (t1.nanosec != t2.nanosec)
    {
        return false;  // 나노초 값이 다르면 false
    }
    return true;  // 모든 값이 같으면 true
}

/**
 * 두 Time_t 객체가 다른지 비교하는 연산자.
 * @param t1 비교할 첫 번째 Time_t 객체
 * @param t2 비교할 두 번째 Time_t 객체
 * @return 두 객체가 다르면 true, 아니면 false 반환
 */
static inline bool operator !=(
        const Time_t& t1,
        const Time_t& t2)
{
    if (t1.seconds != t2.seconds)
    {
        return true;  // 초 값이 다르면 true
    }
    if (t1.nanosec != t2.nanosec)
    {
        return true;  // 나노초 값이 다르면 true
    }
    return false;  // 모든 값이 같으면 false
}

/**
 * 첫 번째 Time_t 객체가 두 번째 객체보다 작은지 비교하는 연산자.
 * @param t1 비교할 첫 번째 Time_t 객체
 * @param t2 비교할 두 번째 Time_t 객체
 * @return 첫 번째 객체가 작으면 true, 아니면 false 반환
 */
static inline bool operator <(
        const Time_t& t1,
        const Time_t& t2)
{
    if (t1.seconds < t2.seconds)
    {
        return true;  // 초 값이 작으면 true
    }
    else if (t1.seconds > t2.seconds)
    {
        return false;  // 초 값이 크면 false
    }
    else
    {
        if (t1.nanosec < t2.nanosec)
        {
            return true;  // 초 값이 같고 나노초 값이 작으면 true
        }
        else
        {
            return false;  // 나노초 값이 크면 false
        }
    }
}

/**
 * 첫 번째 Time_t 객체가 두 번째 객체보다 큰지 비교하는 연산자.
 * @param t1 비교할 첫 번째 Time_t 객체
 * @param t2 비교할 두 번째 Time_t 객체
 * @return 첫 번째 객체가 크면 true, 아니면 false 반환
 */
static inline bool operator >(
        const Time_t& t1,
        const Time_t& t2)
{
    if (t1.seconds > t2.seconds)
    {
        return true;  // 초 값이 크면 true
    }
    else if (t1.seconds < t2.seconds)
    {
        return false;  // 초 값이 작으면 false
    }
    else
    {
        if (t1.nanosec > t2.nanosec)
        {
            return true;  // 초 값이 같고 나노초 값이 크면 true
        }
        else
        {
            return false;  // 나노초 값이 작으면 false
        }
    }
}

/**
 * 첫 번째 Time_t 객체가 두 번째 객체보다 작거나 같은지 비교하는 연산자.
 * @param t1 비교할 첫 번째 Time_t 객체
 * @param t2 비교할 두 번째 Time_t 객체
 * @return 첫 번째 객체가 작거나 같으면 true, 아니면 false 반환
 */
static inline bool operator <=(
        const Time_t& t1,
        const Time_t& t2)
{
    if (t1.seconds < t2.seconds)
    {
        return true;  // 초 값이 작으면 true
    }
    else if (t1.seconds > t2.seconds)
    {
        return false;  // 초 값이 크면 false
    }
    else
    {
        if (t1.nanosec <= t2.nanosec)
        {
            return true;  // 초 값이 같고 나노초 값이 작거나 같으면 true
        }
        else
        {
            return false;  // 나노초 값이 크면 false
        }
    }
}

/**
 * 첫 번째 Time_t 객체가 두 번째 객체보다 크거나 같은지 비교하는 연산자.
 * @param t1 비교할 첫 번째 Time_t 객체
 * @param t2 비교할 두 번째 Time_t 객체
 * @return 첫 번째 객체가 크거나 같으면 true, 아니면 false 반환
 */
static inline bool operator >=(
        const Time_t& t1,
        const Time_t& t2)
{
    if (t1.seconds > t2.seconds)
    {
        return true;  // 초 값이 크면 true
    }
    else if (t1.seconds < t2.seconds)
    {
        return false;  // 초 값이 작으면 false
    }
    else
    {
        if (t1.nanosec >= t2.nanosec)
        {
            return true;  // 초 값이 같고 나노초 값이 크거나 같으면 true
        }
        else
        {
            return false;  // 나노초 값이 작으면 false
        }
    }
}

// Time_t 객체를 출력 스트림에 출력하기 위한 연산자 오버로드
inline std::ostream& operator <<(
        std::ostream& output,
        const Time_t& t)
{
    // 초와 나노초를 결합하여 실수(long double)형태의 시간값 계산 (나노초를 10^9로 나눔)
    long double t_aux = t.seconds + (((long double)t.nanosec) / 1000000000ULL);
    return output << t_aux;  // 계산된 시간값을 출력 스트림에 삽입
}

/**
 * 두 Time_t 객체의 덧셈을 수행하는 연산자 오버로드.
 * @param ta 첫 번째 Time_t 객체
 * @param tb 두 번째 Time_t 객체
 * @return 덧셈 결과를 나타내는 새로운 Time_t 객체 반환
 */
static inline Time_t operator +(
        const Time_t& ta,
        const Time_t& tb)
{
    // seconds와 nanosec 값을 각각 더한 결과로 임시 Time_t 객체 생성
    Time_t result(ta.seconds + tb.seconds, ta.nanosec + tb.nanosec);
    if (result.nanosec < ta.nanosec) // 나노초 덧셈 중 오버플로우 발생 시
    {
        ++result.seconds; // 초 값을 1 증가시킴
    }
    return result;  // 결과 객체 반환
}

/**
 * 두 Time_t 객체의 뺄셈을 수행하는 연산자 오버로드.
 * @param ta 피감수(Time_t 객체, 빼기 대상)
 * @param tb 감수(Time_t 객체, 빼줄 값)
 * @return 뺄셈 결과를 나타내는 새로운 Time_t 객체 반환
 */
static inline Time_t operator -(
        const Time_t& ta,
        const Time_t& tb)
{
    // seconds와 nanosec 값을 각각 뺀 결과로 임시 Time_t 객체 생성
    Time_t result(ta.seconds - tb.seconds, ta.nanosec - tb.nanosec);
    if (result.nanosec > ta.nanosec) // 나노초 뺄셈 중 언더플로우 발생 시
    {
        --result.seconds; // 초 값을 1 감소시킴
    }
    return result;  // 결과 객체 반환
}

#endif // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

//! Time_t (dds::Duration_t)로서 무한대 시간을 나타내는 상수 (생성자에서는 사용하지 말것)
const Time_t c_TimeInfinite{Time_t::INFINITE_SECONDS, Time_t::INFINITE_NANOSECONDS};
//! Time_t (dds::Duration_t)로서 0 시간을 나타내는 상수 (생성자에서는 사용하지 말것)
const Time_t c_TimeZero{0, 0};
//! Time_t (dds::Duration_t)로서 유효하지 않은 시간을 나타내는 상수 (생성자에서는 사용하지 말것)
const Time_t c_TimeInvalid{-1, Time_t::INFINITE_NANOSECONDS};

} // namespace dds    // dds 네임스페이스 종료
} // namespace fastdds  // fastdds 네임스페이스 종료
} // namespace eprosima  // eprosima 네임스페이스 종료

#endif // FASTDDS_DDS_CORE__TIME_T_HPP  // 헤더 파일 다중 포함 방지 매크로 종료