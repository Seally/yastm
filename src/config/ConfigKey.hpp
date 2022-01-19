#pragma once

#include <functional>
#include <string>

#include <fmt/format.h>

enum class BoolConfigKey {
    AllowPartiallyFillingSoulGems,
    AllowSoulDisplacement,
    AllowSoulRelocation,
    AllowExtraSoulRelocation,
    AllowSoulDiversion,
    PreserveOwnership,
    AllowNotifications,
    AllowProfiling,
    Count,
};

inline constexpr std::string_view toString(const BoolConfigKey key) noexcept
{
    using namespace std::literals;

    switch (key) {
    case BoolConfigKey::AllowPartiallyFillingSoulGems:
        return "allowPartiallyFillingSoulGems"sv;
    case BoolConfigKey::AllowSoulDisplacement:
        return "allowSoulDisplacement"sv;
    case BoolConfigKey::AllowSoulRelocation:
        return "allowSoulRelocation"sv;
    case BoolConfigKey::AllowExtraSoulRelocation:
        return "allowExtraSoulRelocation"sv;
    case BoolConfigKey::AllowSoulDiversion:
        return "allowSoulDiversion"sv;
    case BoolConfigKey::PreserveOwnership:
        return "preserveOwnership"sv;
    case BoolConfigKey::AllowNotifications:
        return "allowNotifications"sv;
    case BoolConfigKey::AllowProfiling:
        return "allowProfiling"sv;
    case BoolConfigKey::Count:
        return "<count>"sv;
    }

    return "<invalid BoolConfigKey>"sv;
}

/**
 * @brief Calls fn(configKey, defaultValue) for each available configuration
 * key.
 */
inline void
    forEachBoolConfigKey(const std::function<void(BoolConfigKey, bool)>& fn)
{
    fn(BoolConfigKey::AllowPartiallyFillingSoulGems, 1);
    fn(BoolConfigKey::AllowSoulDisplacement, 1);
    fn(BoolConfigKey::AllowSoulRelocation, 1);
    fn(BoolConfigKey::AllowExtraSoulRelocation, 1);
    fn(BoolConfigKey::AllowSoulDiversion, 0);
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
    fn(BoolConfigKey::AllowExtraSoulRelocation);
    fn(BoolConfigKey::AllowSoulDiversion);
    fn(BoolConfigKey::PreserveOwnership);
    fn(BoolConfigKey::AllowNotifications);
    fn(BoolConfigKey::AllowProfiling);
}

enum class EnumConfigKey { SoulShrinkingTechnique, Count };

/**
 * @brief All enum config value enums should use this as the underlying type so
 * they can be stored without needing variants.
 */
using EnumConfigUnderlyingType = int;

enum class SoulShrinkingTechnique : EnumConfigUnderlyingType {
    None,
    Shrink,
    Split,
};

inline constexpr std::string_view toString(const EnumConfigKey key) noexcept
{
    using namespace std::literals;

    switch (key) {
    case EnumConfigKey::SoulShrinkingTechnique:
        return "soulShrinkingTechnique"sv;
    case EnumConfigKey::Count:
        return "<count>"sv;
    }

    return "<invalid EnumConfigKey>"sv;
}

inline void
    forEachEnumConfigKey(const std::function<void(EnumConfigKey, float)>& fn)
{
    fn(EnumConfigKey::SoulShrinkingTechnique,
       static_cast<float>(SoulShrinkingTechnique::Shrink));
}

inline void forEachEnumConfigKey(const std::function<void(EnumConfigKey)>& fn)
{
    fn(EnumConfigKey::SoulShrinkingTechnique);
}

inline constexpr std::string_view
    toString(const SoulShrinkingTechnique key) noexcept
{
    using namespace std::literals;

    switch (key) {
    case SoulShrinkingTechnique::None:
        return "none"sv;
    case SoulShrinkingTechnique::Shrink:
        return "shrink"sv;
    case SoulShrinkingTechnique::Split:
        return "split"sv;
    }

    return ""sv;
}

template <EnumConfigKey>
struct EnumConfigKeyTypeMap;

template <>
struct EnumConfigKeyTypeMap<EnumConfigKey::SoulShrinkingTechnique> {
    using type = SoulShrinkingTechnique;

    type operator()(const float value) noexcept
    {
        if (value == static_cast<float>(type::Shrink)) {
            return type::Shrink;
        }

        if (value == static_cast<float>(type::Split)) {
            return type::Split;
        }

        return type::None;
    }
};

template <>
struct fmt::formatter<BoolConfigKey> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
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
            throw format_error("invalid format");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const BoolConfigKey key, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        return format_to(ctx.out(), toString(key));
    }
};

template <>
struct fmt::formatter<EnumConfigKey> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
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
            throw format_error("invalid format");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const EnumConfigKey key, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        return format_to(ctx.out(), toString(key));
    }
};

template <>
struct fmt::formatter<SoulShrinkingTechnique> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
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
            throw format_error("invalid format");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const SoulShrinkingTechnique key, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        return format_to(ctx.out(), toString(key));
    }
};
