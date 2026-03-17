//
// Created by SyperOlao on 17.03.2026.
//

#include "Timer.h"
#include <Windows.h>
#include <stdexcept>

namespace {
    [[nodiscard]] long long QueryCounter() {
        LARGE_INTEGER value{};
        if (!QueryPerformanceCounter(&value)) {
            throw std::runtime_error("QueryPerformanceCounter failed");
        }

        return value.QuadPart;
    }

    [[nodiscard]] long long QueryFrequency() {
        LARGE_INTEGER value{};
        if (!QueryPerformanceFrequency(&value)) {
            throw std::runtime_error("QueryPerformanceFrequency failed");
        }

        return value.QuadPart;
    }
}

Timer::Timer()
    : m_frequency(QueryFrequency()) {
    Reset();
}

void Timer::Reset() {
    m_startCounter = QueryCounter();
    m_previousCounter = m_startCounter;
    m_currentCounter = m_startCounter;
    m_deltaTime = 0.0f;
}

void Timer::Tick() {
    m_currentCounter = QueryCounter();

    const auto elapsedCounts = m_currentCounter - m_previousCounter;
    m_deltaTime = static_cast<float>(static_cast<double>(elapsedCounts) / static_cast<double>(m_frequency));

    m_previousCounter = m_currentCounter;
}

float Timer::GetDeltaTime() const noexcept {
    return m_deltaTime;
}

double Timer::GetTotalTime() const noexcept {
    const auto elapsedCounts = m_currentCounter - m_startCounter;
    return static_cast<double>(elapsedCounts) / static_cast<double>(m_frequency);
}
