/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ETimer.h: timer to get FPS
/*=============================================================================*/
#ifndef TIMER_HPP
#define	TIMER_HPP

//Standard includes
#include <cstdint>

class Timer final
{
public:
    Timer();
    ~Timer() = default;

    DEL_ROF(Timer)

    void Reset();
    void Start();
    void Update();
    void Stop();

    void PrintFPS() const noexcept;
    
    [[nodiscard]] constexpr auto GetFPS() const noexcept -> uint32_t { return m_FPS; }
    [[nodiscard]] constexpr auto GetElapsed() const noexcept -> float { return m_ElapsedTime; }
    [[nodiscard]] constexpr auto GetTotal() const noexcept -> float { return m_TotalTime; }
    [[nodiscard]] constexpr auto IsRunning() const noexcept -> bool { return !m_IsStopped; }

private:
    uint64_t m_BaseTime;
    uint64_t m_PausedTime;
    uint64_t m_StopTime;
    uint64_t m_PreviousTime;
    uint64_t m_CurrentTime;

    uint32_t m_FPS;
    uint32_t m_FPSCount;

    float m_TotalTime;
    float m_ElapsedTime;
    float m_SecondsPerCount;
    float m_ElapsedUpperBound;
    float m_FPSTimer;

    bool m_IsStopped;
    bool m_ForceElapsedUpperBound;
};

#endif // !TIMER_HPP
