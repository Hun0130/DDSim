#ifndef SIMULATION_TIME_MANAGER_HPP
#define SIMULATION_TIME_MANAGER_HPP

/**
 * @brief 최소한의 시뮬레이션 시간 구조체입니다.
 *
 * 이 구조체는 시뮬레이션 시간(나노초 단위)을 관리하며,
 * 전체 나노초 값을 반환하는 `getNanoseconds()` 함수와 시간을 진전시키는 `advance()` 함수를 제공합니다.
 */
struct SimTime
{
    int64_t nanoseconds;  // 나노초 단위로 저장되는 시간 값

    // 기본 생성자: 시간을 0으로 초기화
    SimTime(int64_t ns = 0)
        : nanoseconds(ns)
    {
    }

    // 현재 시간을 나노초 단위로 반환
    int64_t getNanoseconds() const
    {
        return nanoseconds;
    }

    // 지정된 나노초만큼 시간을 증가시킴
    void advance(int64_t ns)
    {
        nanoseconds += ns;
    }
};

/**
 * @brief 전역 시뮬레이션 시간을 관리하는 싱글톤 클래스
 *
 * 이 클래스는 시뮬레이터 내에서 글로벌 시뮬레이션 시간을 저장하며,
 * 외부에서 현재 시간을 조회하거나 설정, 또는 특정 시간만큼 증가시킬 수 있는 기능을 제공합니다.
 */
class SimulationTimeManager
{
public:
    // 싱글톤 인스턴스를 반환
    static SimulationTimeManager& instance()
    {
        static SimulationTimeManager instance;
        return instance;
    }

    // 현재 시뮬레이션 시간을 반환
    SimTime getCurrentTime() const
    {
        return current_time;
    }

    // 외부에서 현재 시뮬레이션 시간을 설정
    void setCurrentTime(const SimTime& time)
    {
        current_time = time;
    }

    // 시뮬레이션 시간을 진전시키는 함수 (나노초 단위)
    void advanceTime(int64_t nanoseconds)
    {
        current_time.advance(nanoseconds);
    }

private:
    SimulationTimeManager() = default;            // 생성자: 외부에서 직접 생성 불가 (싱글톤)
    ~SimulationTimeManager() = default;           // 소멸자: 기본 소멸자
    SimulationTimeManager(const SimulationTimeManager&) = delete;   // 복사 생성자 금지
    SimulationTimeManager& operator=(const SimulationTimeManager&) = delete; // 대입 연산자 금지

    // 내부에서 관리하는 현재 시뮬레이션 시간
    SimTime current_time{0};
};

#endif // SIMULATION_TIME_MANAGER_HPP