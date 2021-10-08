#pragma once

#include <functional>
#include <string>

#include <fmt/format.h>

enum class ConfigKey {
    AllowPartiallyFillingSoulGems,
    AllowSoulDisplacement,
    AllowSoulRelocation,
    AllowSoulShrinking,
    AllowSoulSplitting,
    AllowExtraSoulRelocation,
    PreserveOwnership,
    AllowNotifications,
    AllowProfiling,
};

inline constexpr std::string_view toString(const ConfigKey key)
{
    using namespace std::literals;

    switch (key) {
    case ConfigKey::AllowPartiallyFillingSoulGems:
        return "allowPartiallyFillingSoulGems"sv;
    case ConfigKey::AllowSoulDisplacement:
        return "allowSoulDisplacement"sv;
    case ConfigKey::AllowSoulRelocation:
        return "allowSoulRelocation"sv;
    case ConfigKey::AllowSoulShrinking:
        return "allowSoulShrinking"sv;
    case ConfigKey::AllowSoulSplitting:
        return "allowSoulSplitting"sv;
    case ConfigKey::AllowExtraSoulRelocation:
        return "allowExtraSoulRelocation"sv;
    case ConfigKey::PreserveOwnership:
        return "preserveOwnership"sv;
    case ConfigKey::AllowNotifications:
        return "allowNotifications"sv;
    case ConfigKey::AllowProfiling:
        return "allowProfiling"sv;
    }

    return ""sv;
}

/**
 * @brief Calls fn(configKey) for each available configuration key.
 */
inline void forEachConfigKey(const std::function<void(ConfigKey)>& fn)
{
    fn(ConfigKey::AllowPartiallyFillingSoulGems);
    fn(ConfigKey::AllowSoulDisplacement);
    fn(ConfigKey::AllowSoulRelocation);
    fn(ConfigKey::AllowSoulShrinking);
    fn(ConfigKey::AllowSoulSplitting);
    fn(ConfigKey::AllowExtraSoulRelocation);
    fn(ConfigKey::PreserveOwnership);
    fn(ConfigKey::AllowNotifications);
    fn(ConfigKey::AllowProfiling);
}

/**
 * @brief Calls fn(configKey, defaultValue) for each available configuration
 * key.
 */
inline void forEachConfigKey(const std::function<void(ConfigKey, bool)>& fn)
{
    fn(ConfigKey::AllowPartiallyFillingSoulGems, 1);
    fn(ConfigKey::AllowSoulDisplacement, 1);
    fn(ConfigKey::AllowSoulRelocation, 1);
    fn(ConfigKey::AllowSoulShrinking, 1);
    fn(ConfigKey::AllowSoulSplitting, 0);
    fn(ConfigKey::AllowExtraSoulRelocation, 1);
    fn(ConfigKey::PreserveOwnership, 1);
    fn(ConfigKey::AllowNotifications, 1);
    fn(ConfigKey::AllowProfiling, 0);
}

template <>
struct fmt::formatter<ConfigKey> {
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
    auto format(const ConfigKey key, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        return format_to(ctx.out(), toString(key));
    }
};
