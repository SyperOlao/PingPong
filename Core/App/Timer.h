//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_TIMER_H
#define PINGPONG_TIMER_H

class Timer
{
public:
    Timer();

    void Reset();
    void Tick();

    [[nodiscard]] float GetDeltaTime() const noexcept;
    [[nodiscard]] double GetTotalTime() const noexcept;

private:
    long long m_frequency {};
    long long m_startCounter {};
    long long m_previousCounter {};
    long long m_currentCounter {};

    float m_deltaTime {0.0f};
};


#endif //PINGPONG_TIMER_H