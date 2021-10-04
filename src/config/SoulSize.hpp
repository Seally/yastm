#ifndef CONFIG_SOULSIZE_HPP
#define CONFIG_SOULSIZE_HPP

#include <type_traits>

enum class SoulSize {
    None = 0,
    Petty = 1,
    Lesser = 2,
    Common = 3,
    Greater = 4,
    Grand = 5,
    Black = 6,
};

enum class RawSoulSize {
    None = 0,
    Petty = 250,
    Lesser = 500,
    Common = 1000,
    Greater = 2000,
    Grand = 3000,
};

inline constexpr RawSoulSize toRawSoulSize(const SoulSize soulSize) {
    switch (soulSize) {
    case SoulSize::None:
        return RawSoulSize::None;
    case SoulSize::Petty:
        return RawSoulSize::Petty;
    case SoulSize::Lesser:
        return RawSoulSize::Lesser;
    case SoulSize::Common:
        return RawSoulSize::Common;
    case SoulSize::Greater:
        return RawSoulSize::Greater;
    case SoulSize::Grand:
    case SoulSize::Black:
        return RawSoulSize::Grand;
    }

    return RawSoulSize::None;
}

inline constexpr SoulSize
    toSoulSize(const RawSoulSize rawSoulSize, const bool isNpc = false)
{
    if (isNpc) {
        return SoulSize::Black;
    }

    switch (rawSoulSize) {
    case RawSoulSize::None:
        return SoulSize::None;
    case RawSoulSize::Petty:
        return SoulSize::Petty;
    case RawSoulSize::Lesser:
        return SoulSize::Lesser;
    case RawSoulSize::Common:
        return SoulSize::Common;
    case RawSoulSize::Greater:
        return SoulSize::Greater;
    case RawSoulSize::Grand:
        return SoulSize::Grand;
    }

    return SoulSize::None;
}

template <typename T>
inline constexpr T operator+(const SoulSize soulSize, const T other)
{
    static_assert(std::is_integral_v<T>);
    static_assert(!std::is_same_v<SoulSize, T>);

    return static_cast<T>(soulSize) + other;
}

template <typename T>
inline constexpr T operator+(const T other, const SoulSize soulSize)
{
    static_assert(std::is_integral_v<T>);
    static_assert(!std::is_same_v<SoulSize, T>);

    return other + static_cast<T>(soulSize);
}

template <typename T>
inline constexpr T operator-(const SoulSize soulSize, const T other)
{
    static_assert(std::is_integral_v<T>);
    static_assert(!std::is_same_v<SoulSize, T>);

    return static_cast<T>(soulSize) - other;
}

template <typename T>
inline constexpr T operator-(const T other, const SoulSize soulSize)
{
    static_assert(std::is_integral_v<T>);
    static_assert(!std::is_same_v<SoulSize, T>);

    return other - static_cast<T>(soulSize);
}

template <typename T>
inline constexpr bool operator>(const SoulSize soulSize, const T other)
{
    static_assert(std::is_integral_v<T>);
    static_assert(!std::is_same_v<SoulSize, T>);

    return static_cast<T>(soulSize) > other;
}

template <typename T>
inline constexpr bool operator>(const T other, const SoulSize soulSize)
{
    static_assert(std::is_integral_v<T>);
    static_assert(!std::is_same_v<SoulSize, T>);

    return other > static_cast<T>(soulSize);
}

template <typename T>
inline constexpr bool operator<(const SoulSize soulSize, const T other)
{
    static_assert(std::is_integral_v<T>);
    static_assert(!std::is_same_v<SoulSize, T>);

    return static_cast<T>(soulSize) < other;
}

template <typename T>
inline constexpr bool operator<(const T other, const SoulSize soulSize)
{
    static_assert(std::is_integral_v<T>);
    static_assert(!std::is_same_v<SoulSize, T>);

    return other < static_cast<T>(soulSize);
}

template <typename T>
inline constexpr bool operator>=(const SoulSize soulSize, const T other)
{
    static_assert(std::is_integral_v<T>);
    static_assert(!std::is_same_v<SoulSize, T>);

    return static_cast<T>(soulSize) >= other;
}

template <typename T>
inline constexpr bool operator>=(const T other, const SoulSize soulSize)
{
    static_assert(std::is_integral_v<T>);
    static_assert(!std::is_same_v<SoulSize, T>);

    return other >= static_cast<T>(soulSize);
}

template <typename T>
inline constexpr bool operator<=(const SoulSize soulSize, const T other)
{
    static_assert(std::is_integral_v<T>);
    static_assert(!std::is_same_v<SoulSize, T>);

    return static_cast<T>(soulSize) <= other;
}

template <typename T>
inline constexpr bool operator<=(const T other, const SoulSize soulSize)
{
    static_assert(std::is_integral_v<T>);
    static_assert(!std::is_same_v<SoulSize, T>);

    return other <= static_cast<T>(soulSize);
}

inline constexpr bool isValidSoulCapacity(const SoulSize soulCapacity)
{
    return SoulSize::Petty <= soulCapacity && soulCapacity <= SoulSize::Black;
}

inline constexpr bool isValidContainedSoulSize(
    const SoulSize soulCapacity,
    const SoulSize containedSoulSize)
{
    if (soulCapacity == SoulSize::Black) {
        return containedSoulSize == SoulSize::None ||
               containedSoulSize == SoulSize::Black;
    }

    return SoulSize::None <= containedSoulSize &&
           containedSoulSize <= soulCapacity;
}

inline constexpr std::size_t
    getVariantCountForCapacity(const SoulSize soulCapacity)
{
    // Black soul gems only need 2 variants: filled and unfilled.
    if (soulCapacity == SoulSize::Black) {
        return 2;
    }

    return static_cast<std::size_t>(soulCapacity) + 1;
}

#endif // CONFIG_SOULSIZE_HPP
