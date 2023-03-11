#pragma once

#include <concepts>
#include <random>
#include <type_traits>

class Rng {
    std::mt19937 engine_;

    explicit Rng()
        : engine_(std::random_device()())
    {}

    Rng(const Rng&) = delete;
    Rng(Rng&&) = delete;
    Rng& operator=(const Rng&) = delete;
    Rng& operator=(Rng&&) = delete;

public:
    static Rng& getInstance()
    {
        static Rng instance;
        return instance;
    }

    template <typename T>
    requires std::integral<T> || std::floating_point<T>
        T generateUniform(T min, T max)
    {
        if constexpr (std::is_integral_v<T>) {
            std::uniform_int_distribution<T> dist(min, max);

            return dist(engine_);
        } else {
            std::uniform_real_distribution<T> dist(min, max);

            return dist(engine_);
        }
    }
};
