#pragma once

#include <functional>
#include <string>

#include <fmt/format.h>

enum class LoadPriority {
    /**
     * @brief Soul gem group are added to the list according to the group's soul
     * gem type.
     */
    Auto,
    /**
     * @brief These soul gem groups are added to the list first. When set to
     * AUTO, this is the load priority for reusable soul gems.
     */
    High,
    /**
     * @brief These soul gem groups are the second set added to the soul gem
     * map. When set to AUTO, this is the load priority for non-reusable
     * soul gems.
     */
    Normal,
    /**
     * @brief These soul gem groups are the last ones added to the soul gem map.
     * This priority level is only specified explicitly by the user.
     */
    Low,
    Invalid,
};

inline std::string_view toString(const LoadPriority priority) noexcept
{
    switch (priority) {
    case LoadPriority::Auto:
        return "auto"sv;
    case LoadPriority::High:
        return "high"sv;
    case LoadPriority::Normal:
        return "normal"sv;
    case LoadPriority::Low:
        return "low"sv;
    }

    return "<invalid load priority>"sv;
}

inline LoadPriority fromLoadPriorityString(std::string_view str) noexcept
{
    if (str == "auto"sv) {
        return LoadPriority::Auto;
    }

    if (str == "high"sv) {
        return LoadPriority::High;
    }

    if (str == "normal"sv) {
        return LoadPriority::Normal;
    }

    if (str == "low"sv) {
        return LoadPriority::Low;
    }

    return LoadPriority::Invalid;
}

template <>
struct fmt::formatter<LoadPriority> {
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
        auto it = ctx.begin(), end = ctx.end();

        // Check if reached the end of the range:
        if (it != end && *it != '}') {
            throw fmt::format_error("invalid format");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const LoadPriority key, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), fmt::runtime(toString(key)));
    }
};

/**
 * @brief Calls fn(loadPriority) for each available non-nominal load priority
 * (skips LoadPriority::Auto and LoadPriority::Invalid).
 */
inline void forEachLoadPriority(const std::function<void(LoadPriority)>& fn)
{
    fn(LoadPriority::High);
    fn(LoadPriority::Normal);
    fn(LoadPriority::Low);
}
