#pragma once

#include <chrono>

class Timer {
    using clock_type = std::chrono::steady_clock;
    using second_type = std::chrono::duration<double, std::ratio<1>>;

    std::chrono::time_point<clock_type> _begin;

public:
    explicit Timer()
        : _begin(clock_type::now())
    {}
    virtual ~Timer() {}

    void reset() { _begin = clock_type::now(); }

    double elapsed() const
    {
        return std::chrono::duration_cast<second_type>(
                   clock_type::now() - _begin)
            .count();
    }
};

/**
 * @brief A timer that can accumulate multiple time periods. Note that, in order
 * to minimize the performance impact, this is very low-level and can be
 * error-prone. Make sure to always match stopPeriod() with a call to
 * startPeriod().
 */
class AccumulatingTimer {
    using clock_type = std::chrono::steady_clock;
    using second_type = std::chrono::duration<double, std::ratio<1>>;

    second_type _totalDuration;
    std::chrono::time_point<clock_type> _begin;

public:
    explicit AccumulatingTimer()
        : _totalDuration(second_type::zero())
    {}
    virtual ~AccumulatingTimer() {}

    /**
     * @brief Sets the total duration to zero.
     */
    void reset() { _totalDuration = second_type::zero(); }

    /**
     * @brief Starts a timer period.
     */
    void startPeriod() { _begin = clock_type::now(); }

    /**
     * @brief Adds the time from the last timer start to the total duration.
     */
    void stopPeriod()
    {
        _totalDuration +=
            std::chrono::duration_cast<second_type>(clock_type::now() - _begin);
    }

    /**
     * @brief Returns the accumulated time. Does not include the time from the
     * current period. 
     */
    double elapsed() const { return _totalDuration.count(); }
};
