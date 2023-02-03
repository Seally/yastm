#pragma once

#include <concepts>
#include <exception>
#include <functional>

#include <fmt/format.h>

#include <RE/S/SoulLevels.h>

#include "../utilities/stringutils.hpp"

enum class SoulSize {
    None,
    Petty,
    Lesser,
    Common,
    Greater,
    Grand,
    Black,
    Size,
    First = None,
    Last = Black,
    LastWhite = Grand,
};

enum class SoulLevelValue {
    None = 0,
    Petty = 250,
    Lesser = 500,
    Common = 1000,
    Greater = 2000,
    Grand = 3000,
};

enum class SoulGemCapacity {
    Petty,
    Lesser,
    Common,
    Greater,
    Grand,
    Dual,
    Black,
    Size,
    First = Petty,
    Last = Black,
    LastWhite = Dual,
};

[[nodiscard]] constexpr SoulSize toSoulSize(const RE::SOUL_LEVEL soulLevel)
{
    switch (soulLevel) {
    case RE::SOUL_LEVEL::kNone:
        return SoulSize::None;
    case RE::SOUL_LEVEL::kPetty:
        return SoulSize::Petty;
    case RE::SOUL_LEVEL::kLesser:
        return SoulSize::Lesser;
    case RE::SOUL_LEVEL::kCommon:
        return SoulSize::Common;
    case RE::SOUL_LEVEL::kGreater:
        return SoulSize::Greater;
    case RE::SOUL_LEVEL::kGrand:
        return SoulSize::Grand;
    }

    throw std::runtime_error(fmt::format(
        FMT_STRING("Invalid RE::SOUL_LEVEL value: {}"),
        static_cast<std::underlying_type_t<RE::SOUL_LEVEL>>(soulLevel)));
}

[[nodiscard]] constexpr SoulSize toSoulSize(const SoulGemCapacity capacity)
{
    switch (capacity) {
    case SoulGemCapacity::Petty:
        return SoulSize::Petty;
    case SoulGemCapacity::Lesser:
        return SoulSize::Lesser;
    case SoulGemCapacity::Common:
        return SoulSize::Common;
    case SoulGemCapacity::Greater:
        return SoulSize::Greater;
    case SoulGemCapacity::Grand:
    case SoulGemCapacity::Dual:
        return SoulSize::Grand;
    case SoulGemCapacity::Black:
        return SoulSize::Black;
    }

    throw std::runtime_error(fmt::format(
        FMT_STRING("Invalid SoulGemCapacity value: {}"),
        static_cast<std::underlying_type_t<SoulGemCapacity>>(capacity)));
}

[[nodiscard]] constexpr SoulLevelValue toSoulLevelValue(const SoulSize soulSize)
{
    switch (soulSize) {
    case SoulSize::None:
        return SoulLevelValue::None;
    case SoulSize::Petty:
        return SoulLevelValue::Petty;
    case SoulSize::Lesser:
        return SoulLevelValue::Lesser;
    case SoulSize::Common:
        return SoulLevelValue::Common;
    case SoulSize::Greater:
        return SoulLevelValue::Greater;
    case SoulSize::Grand:
    case SoulSize::Black:
        return SoulLevelValue::Grand;
    }

    throw std::runtime_error(fmt::format(
        FMT_STRING("Invalid SoulSize value: {}"),
        static_cast<std::underlying_type_t<SoulSize>>(soulSize)));
}

[[nodiscard]] inline constexpr RE::SOUL_LEVEL
    toSoulLevel(const SoulSize soulSize)
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

    throw std::runtime_error(fmt::format(
        FMT_STRING("Invalid SoulSize value: {}"),
        static_cast<std::underlying_type_t<SoulSize>>(soulSize)));
}

[[nodiscard]] constexpr RE::SOUL_LEVEL
    toSoulLevel(const SoulGemCapacity capacity)
{
    switch (capacity) {
    case SoulGemCapacity::Petty:
        return RE::SOUL_LEVEL::kPetty;
    case SoulGemCapacity::Lesser:
        return RE::SOUL_LEVEL::kLesser;
    case SoulGemCapacity::Common:
        return RE::SOUL_LEVEL::kCommon;
    case SoulGemCapacity::Greater:
        return RE::SOUL_LEVEL::kGreater;
    case SoulGemCapacity::Grand:
    case SoulGemCapacity::Dual:
    case SoulGemCapacity::Black:
        return RE::SOUL_LEVEL::kGrand;
    }

    throw std::runtime_error(fmt::format(
        FMT_STRING("Invalid SoulGemCapacity value: {}"),
        static_cast<std::underlying_type_t<SoulGemCapacity>>(capacity)));
}

[[nodiscard]] constexpr SoulGemCapacity
    toSoulGemCapacity(const SoulSize soulSize)
{
    switch (soulSize) {
    case SoulSize::None:
        throw std::runtime_error(
            "Cannot convert SoulSize::None to SoulGemCapacity");
    case SoulSize::Petty:
        return SoulGemCapacity::Petty;
    case SoulSize::Lesser:
        return SoulGemCapacity::Lesser;
    case SoulSize::Common:
        return SoulGemCapacity::Common;
    case SoulSize::Greater:
        return SoulGemCapacity::Greater;
    case SoulSize::Grand:
        return SoulGemCapacity::Grand;
    case SoulSize::Black:
        return SoulGemCapacity::Black;
    }

    throw std::runtime_error(fmt::format(
        FMT_STRING("Invalid SoulSize value: {}"),
        static_cast<std::underlying_type_t<SoulSize>>(soulSize)));
}

[[nodiscard]] constexpr const char* toString(const SoulSize soulSize)
{
    switch (soulSize) {
    case SoulSize::None:
        return "none";
    case SoulSize::Petty:
        return "petty";
    case SoulSize::Lesser:
        return "lesser";
    case SoulSize::Common:
        return "common";
    case SoulSize::Greater:
        return "greater";
    case SoulSize::Grand:
        return "grand";
    case SoulSize::Black:
        return "black";
    }

    throw std::runtime_error(fmt::format(
        FMT_STRING("Invalid SoulSize value: {}"),
        static_cast<std::underlying_type_t<SoulSize>>(soulSize)));
}

[[nodiscard]] constexpr const char* toString(const SoulGemCapacity capacity)
{
    switch (capacity) {
    case SoulGemCapacity::Petty:
        return "petty";
    case SoulGemCapacity::Lesser:
        return "lesser";
    case SoulGemCapacity::Common:
        return "common";
    case SoulGemCapacity::Greater:
        return "greater";
    case SoulGemCapacity::Grand:
        return "grand";
    case SoulGemCapacity::Dual:
        return "dual";
    case SoulGemCapacity::Black:
        return "black";
    }

    // Avoid using the formatter since the formatter also calls this function,
    // potentially leading to infinite recursion.
    throw std::runtime_error(fmt::format(
        FMT_STRING("Invalid SoulGemCapacity value: {}"),
        static_cast<std::underlying_type_t<SoulGemCapacity>>(capacity)));
}

inline void forEachSoulGemCapacity(std::function<void(SoulGemCapacity)> fn)
{
    fn(SoulGemCapacity::Petty);
    fn(SoulGemCapacity::Lesser);
    fn(SoulGemCapacity::Common);
    fn(SoulGemCapacity::Greater);
    fn(SoulGemCapacity::Grand);
    fn(SoulGemCapacity::Dual);
    fn(SoulGemCapacity::Black);
}

// -----------------------------------------------------------------------------
// SoulSize operator overloads
// -----------------------------------------------------------------------------
template <typename T>
    requires std::integral<T>
constexpr T operator+(const SoulSize soulSize, const T other)
{
    return static_cast<T>(soulSize) + other;
}

template <typename T>
    requires std::integral<T>
constexpr T operator+(const T other, const SoulSize soulSize)
{
    return other + static_cast<T>(soulSize);
}

template <typename T>
    requires std::integral<T>
constexpr T operator-(const SoulSize soulSize, const T other)
{
    return static_cast<T>(soulSize) - other;
}

template <typename T>
    requires std::integral<T>
constexpr T operator-(const T other, const SoulSize soulSize)
{
    return other - static_cast<T>(soulSize);
}

// -----------------------------------------------------------------------------
// SoulGemCapacity operator overloads
// -----------------------------------------------------------------------------
template <typename T>
    requires std::integral<T>
constexpr T operator+(const SoulGemCapacity capacity, const T other)
{
    return static_cast<T>(capacity) + other;
}

template <typename T>
    requires std::integral<T>
constexpr T operator+(const T other, const SoulGemCapacity capacity)
{
    return other + static_cast<T>(capacity);
}

template <typename T>
    requires std::integral<T>
constexpr T operator-(const SoulGemCapacity capacity, const T other)
{
    return static_cast<T>(capacity) - other;
}

template <typename T>
    requires std::integral<T>
constexpr T operator-(const T other, const SoulGemCapacity capacity)
{
    return other - static_cast<T>(capacity);
}

// -----------------------------------------------------------------------------
// fmt::format functions
// -----------------------------------------------------------------------------
template <>
struct fmt::formatter<SoulSize> {
private:
    enum class Capitalization {
        AllLower,
        FirstUpper,
    };

    enum class OutputFormat {
        RawNumber,
        Text,
    };

    Capitalization capitalization_ = Capitalization::AllLower;
    OutputFormat outputFormat_ = OutputFormat::RawNumber;

public:
    // Presentation format (in case of conflict, last format character wins):
    //
    // Capitalization:
    //
    //     'L': Display all lowercase (default).
    //     'u': Capitalize first letter.
    //
    // Output format:
    //
    //     'n': Raw number (default).
    //     't': Text
    constexpr auto parse(fmt::format_parse_context& ctx)
        -> decltype(ctx.begin())
    {
        // [ctx.begin(), ctx.end()) is a character range that contains a part of
        // the format string starting from the format specifications to be parsed,
        // e.g. in
        //
        //   fmt::format("{:f} - point of interest", point(1, 2));
        //
        // the range will contain "f} - point of interest". The formatter should
        // parse specifiers until '}' or the end of the range.

        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin();

        for (; it != ctx.end() && *it != '}'; ++it) {
            switch (*it) {
            case 'L':
                capitalization_ = Capitalization::AllLower;
                break;
            case 'u':
                capitalization_ = Capitalization::FirstUpper;
                break;
            case 'n':
                outputFormat_ = OutputFormat::RawNumber;
                break;
            case 't':
                outputFormat_ = OutputFormat::Text;
                break;
            default:
                throw format_error("invalid format");
            }
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const SoulSize soulSize, FormatContext& ctx)
    {
        // ctx.out() is an output iterator to write to.
        std::string formatString;

        if (outputFormat_ == OutputFormat::RawNumber) {
            formatString = std::to_string(
                static_cast<std::underlying_type_t<SoulSize>>(soulSize));
        } else {
            formatString = toString(soulSize);

            if (capitalization_ == Capitalization::FirstUpper) {
                capitalizeFirstChar(formatString);
            }
        }

        return fmt::format_to(ctx.out(), fmt::runtime(formatString));
    }
};

template <>
struct fmt::formatter<SoulLevelValue> : formatter<int> {
    // parse is inherited from formatter<unsigned int>.
    template <typename FormatContext>
    auto format(const SoulLevelValue soulSize, FormatContext& ctx)
    {
        return formatter<int>::format(static_cast<int>(soulSize), ctx);
    }
};

template <>
struct fmt::formatter<SoulGemCapacity> {
private:
    enum class Capitalization {
        AllLower,
        FirstUpper,
    };

    enum class OutputFormat {
        RawNumber,
        Text,
    };

    Capitalization capitalization_ = Capitalization::AllLower;
    OutputFormat outputFormat_ = OutputFormat::RawNumber;

public:
    // Presentation format (in case of conflict, last format character wins):
    //
    // Capitalization:
    //
    //     'L': Display all lowercase (default).
    //     'u': Capitalize first letter.
    //
    // Output format:
    //
    //     'n': Raw number (default).
    //     't': Text
    constexpr auto parse(fmt::format_parse_context& ctx)
        -> decltype(ctx.begin())
    {
        // [ctx.begin(), ctx.end()) is a character range that contains a part of
        // the format string starting from the format specifications to be parsed,
        // e.g. in
        //
        //   fmt::format("{:f} - point of interest", point{1, 2});
        //
        // the range will contain "f} - point of interest". The formatter should
        // parse specifiers until '}' or the end of the range.

        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin();

        for (; it != ctx.end() && *it != '}'; ++it) {
            switch (*it) {
            case 'L':
                capitalization_ = Capitalization::AllLower;
                break;
            case 'u':
                capitalization_ = Capitalization::FirstUpper;
                break;
            case 'n':
                outputFormat_ = OutputFormat::RawNumber;
                break;
            case 't':
                outputFormat_ = OutputFormat::Text;
                break;
            default:
                throw fmt::format_error("invalid format");
            }
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const SoulGemCapacity capacity, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        // ctx.out() is an output iterator to write to.
        std::string formatString;

        if (outputFormat_ == OutputFormat::RawNumber) {
            formatString = std::to_string(
                static_cast<std::underlying_type_t<SoulGemCapacity>>(capacity));
        } else {
            formatString = toString(capacity);

            if (capitalization_ == Capitalization::FirstUpper) {
                capitalizeFirstChar(formatString);
            }
        }

        return fmt::format_to(ctx.out(), fmt::runtime(formatString));
    }
};
