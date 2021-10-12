#pragma once

#include <functional>
#include <string>

#include <fmt/format.h>

enum class BoolConfigKey {
    AllowPartiallyFillingSoulGems,
    AllowSoulDisplacement,
    AllowSoulRelocation,
    AllowSoulShrinking,
    AllowSoulSplitting,
    AllowExtraSoulRelocation,
    AllowSoulDiversion,
    PerformSoulDiversionInDLL,
    PreserveOwnership,
    AllowNotifications,
    AllowProfiling,
    Count,
};

inline constexpr std::string_view toString(const BoolConfigKey key)
{
    using namespace std::literals;

    switch (key) {
    case BoolConfigKey::AllowPartiallyFillingSoulGems:
        return "allowPartiallyFillingSoulGems"sv;
    case BoolConfigKey::AllowSoulDisplacement:
        return "allowSoulDisplacement"sv;
    case BoolConfigKey::AllowSoulRelocation:
        return "allowSoulRelocation"sv;
    case BoolConfigKey::AllowSoulShrinking:
        return "allowSoulShrinking"sv;
    case BoolConfigKey::AllowSoulSplitting:
        return "allowSoulSplitting"sv;
    case BoolConfigKey::AllowExtraSoulRelocation:
        return "allowExtraSoulRelocation"sv;
    case BoolConfigKey::AllowSoulDiversion:
        return "allowSoulDiversion"sv;
    case BoolConfigKey::PerformSoulDiversionInDLL:
        return "performSoulDiversionInDLL";
    case BoolConfigKey::PreserveOwnership:
        return "preserveOwnership"sv;
    case BoolConfigKey::AllowNotifications:
        return "allowNotifications"sv;
    case BoolConfigKey::AllowProfiling:
        return "allowProfiling"sv;
    case BoolConfigKey::Count:
        return "<count>"sv;
    }

    return ""sv;
}

/**
 * @brief Calls fn(configKey, defaultValue) for each available configuration
 * key.
 */
inline void forEachBoolConfigKey(const std::function<void(BoolConfigKey, bool)>& fn)
{
    fn(BoolConfigKey::AllowPartiallyFillingSoulGems, 1);
    fn(BoolConfigKey::AllowSoulDisplacement, 1);
    fn(BoolConfigKey::AllowSoulRelocation, 1);
    fn(BoolConfigKey::AllowSoulShrinking, 1);
    fn(BoolConfigKey::AllowSoulSplitting, 0);
    fn(BoolConfigKey::AllowExtraSoulRelocation, 1);
    fn(BoolConfigKey::AllowSoulDiversion, 0);
    fn(BoolConfigKey::PerformSoulDiversionInDLL, 1);
    fn(BoolConfigKey::PreserveOwnership, 1);
    fn(BoolConfigKey::AllowNotifications, 1);
    fn(BoolConfigKey::AllowProfiling, 0);
}

/**
 * @brief Calls fn(configKey) for each available configuration key.
 */
inline void forEachBoolConfigKey(const std::function<void(BoolConfigKey)>& fn)
{
    fn(BoolConfigKey::AllowPartiallyFillingSoulGems);
    fn(BoolConfigKey::AllowSoulDisplacement);
    fn(BoolConfigKey::AllowSoulRelocation);
    fn(BoolConfigKey::AllowSoulShrinking);
    fn(BoolConfigKey::AllowSoulSplitting);
    fn(BoolConfigKey::AllowExtraSoulRelocation);
    fn(BoolConfigKey::AllowSoulDiversion);
    fn(BoolConfigKey::PerformSoulDiversionInDLL);
    fn(BoolConfigKey::PreserveOwnership);
    fn(BoolConfigKey::AllowNotifications);
    fn(BoolConfigKey::AllowProfiling);
}

template <>
struct fmt::formatter<BoolConfigKey> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
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
        auto it = ctx.begin(), end = ctx.end();

        // Check if reached the end of the range:
        if (it != end && *it != '}') {
            throw format_error("invalid format");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const BoolConfigKey key, FormatContext& ctx) -> decltype(ctx.out())
    {
        return format_to(ctx.out(), toString(key));
    }
};

