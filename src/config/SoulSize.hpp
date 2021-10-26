#pragma once

#include <compare>
#include <concepts>
#include <type_traits>

#include <RE/S/SoulLevels.h>
#include <fmt/format.h>

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

inline constexpr RawSoulSize toRawSoulSize(const SoulSize soulSize) noexcept
{
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

inline constexpr RE::SOUL_LEVEL toSoulLevel(const SoulSize soulSize) noexcept
{
    switch (soulSize) {
    case SoulSize::None:
        return RE::SOUL_LEVEL::kNone;
    case SoulSize::Petty:
        return RE::SOUL_LEVEL::kPetty;
    case SoulSize::Lesser:
        return RE::SOUL_LEVEL::kLesser;
    case SoulSize::Common:
        return RE::SOUL_LEVEL::kCommon;
    case SoulSize::Greater:
        return RE::SOUL_LEVEL::kGreater;
    case SoulSize::Grand:
    case SoulSize::Black:
        return RE::SOUL_LEVEL::kGrand;
    }

    return RE::SOUL_LEVEL::kNone;
}

// -----------------------------------------------------------------------------
// SoulSize operator overloads
// -----------------------------------------------------------------------------
template <typename T>
requires std::integral<T>
inline constexpr T operator+(const SoulSize soulSize, const T other)
{
    static_assert(!std::is_same_v<SoulSize, T>);
    static_assert(!std::is_same_v<RawSoulSize, T>);

    return static_cast<T>(soulSize) + other;
}

template <typename T>
requires std::integral<T>
inline constexpr T operator+(const T other, const SoulSize soulSize)
{
    static_assert(!std::is_same_v<SoulSize, T>);
    static_assert(!std::is_same_v<RawSoulSize, T>);

    return other + static_cast<T>(soulSize);
}

template <typename T>
requires std::integral<T>
inline constexpr T operator-(const SoulSize soulSize, const T other)
{
    static_assert(std::is_integral_v<T>);
    static_assert(!std::is_same_v<SoulSize, T>);
    static_assert(!std::is_same_v<RawSoulSize, T>);

    return static_cast<T>(soulSize) - other;
}

template <typename T>
requires std::integral<T>
inline constexpr T operator-(const T other, const SoulSize soulSize)
{
    static_assert(!std::is_same_v<SoulSize, T>);
    static_assert(!std::is_same_v<RawSoulSize, T>);

    return other - static_cast<T>(soulSize);
}

template <typename T>
requires std::integral<T>
inline constexpr auto operator<=>(const SoulSize soulSize, const T other)
{
    static_assert(!std::is_same_v<SoulSize, T>);
    static_assert(!std::is_same_v<RawSoulSize, T>);

    return static_cast<T>(soulSize) <=> other;
}

template <typename T>
requires std::integral<T>
inline constexpr auto operator==(const SoulSize soulSize, const T other)
{
    static_assert(!std::is_same_v<SoulSize, T>);
    static_assert(!std::is_same_v<RawSoulSize, T>);

    return static_cast<T>(soulSize) == other;
}

// -----------------------------------------------------------------------------
// RawSoulSize operator overloads
// -----------------------------------------------------------------------------
template <typename T>
requires std::integral<T>
inline constexpr T operator+(const RawSoulSize soulSize, const T other)
{
    static_assert(!std::is_same_v<SoulSize, T>);
    static_assert(!std::is_same_v<RawSoulSize, T>);

    return static_cast<T>(soulSize) + other;
}

template <typename T>
requires std::integral<T>
inline constexpr T operator+(const T other, const RawSoulSize soulSize)
{
    static_assert(!std::is_same_v<SoulSize, T>);
    static_assert(!std::is_same_v<RawSoulSize, T>);

    return other + static_cast<T>(soulSize);
}

template <typename T>
requires std::integral<T>
inline constexpr T operator-(const RawSoulSize soulSize, const T other)
{
    static_assert(!std::is_same_v<SoulSize, T>);
    static_assert(!std::is_same_v<RawSoulSize, T>);

    return static_cast<T>(soulSize) - other;
}

template <typename T>
requires std::integral<T>
inline constexpr T operator-(const T other, const RawSoulSize soulSize)
{
    static_assert(!std::is_same_v<SoulSize, T>);
    static_assert(!std::is_same_v<RawSoulSize, T>);

    return other - static_cast<T>(soulSize);
}

template <typename T>
requires std::integral<T>
inline constexpr auto operator<=>(const RawSoulSize soulSize, const T other)
{
    static_assert(!std::is_same_v<SoulSize, T>);
    static_assert(!std::is_same_v<RawSoulSize, T>);

    return static_cast<T>(soulSize) <=> other;
}

template <typename T>
requires std::integral<T>
inline constexpr auto operator==(const RawSoulSize soulSize, const T other)
{
    static_assert(!std::is_same_v<SoulSize, T>);
    static_assert(!std::is_same_v<RawSoulSize, T>);

    return static_cast<T>(soulSize) == other;
}

// -----------------------------------------------------------------------------
// Utility functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// fmt::format functions
// -----------------------------------------------------------------------------
template <>
struct fmt::formatter<SoulSize> : formatter<int> {
    // parse is inherited from formatter<unsigned int>.
    template <typename FormatContext>
    auto format(const SoulSize soulSize, FormatContext& ctx)
    {
        return formatter<int>::format(static_cast<int>(soulSize), ctx);
    }
};

template <>
struct fmt::formatter<RawSoulSize> : formatter<int> {
    // parse is inherited from formatter<unsigned int>.
    template <typename FormatContext>
    auto format(const RawSoulSize soulSize, FormatContext& ctx)
    {
        return formatter<int>::format(static_cast<int>(soulSize), ctx);
    }
};
