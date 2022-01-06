#pragma once

#include <chrono>

class Timer {
    using clock_type = std::chrono::steady_clock;
    using second_type = std::chrono::duration<double, std::ratio<1>>;

    std::chrono::time_point<clock_type> begin_;

public:
    explicit Timer()
        : begin_(clock_type::now())
    {}
    virtual ~Timer() {}

    void reset() { begin_ = clock_type::now(); }

    double elapsed() const
    {
        return std::chrono::duration_cast<second_type>(
                   clock_type::now() - begin_)
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

    second_type totalDuration_;
    std::chrono::time_point<clock_type> begin_;

public:
    explicit AccumulatingTimer()
        : totalDuration_(second_type::zero())
    {}
    virtual ~AccumulatingTimer() {}

    /**
     * @brief Sets the total duration to zero.
     */
    void reset() { totalDuration_ = second_type::zero(); }

    /**
     * @brief Starts a timer period.
     */
    void startPeriod() { begin_ = clock_type::now(); }

    /**
     * @brief Adds the time from the last timer start to the total duration.
     */
    void stopPeriod()
    {
        totalDuration_ +=
            std::chrono::duration_cast<second_type>(clock_type::now() - begin_);
    }

    /**
     * @brief Returns the accumulated time. Does not include the time from the
     * current period. 
     */
    double elapsed() const { return totalDuration_.count(); }
};
