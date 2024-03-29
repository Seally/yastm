#pragma once

#include <functional>
#include <string>

#include <fmt/format.h>

enum class EnumConfigKey {
    SoulShrinkingTechnique,
    SoulTrapLevelingType,
    Count
};

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

enum class SoulTrapLevelingType : EnumConfigUnderlyingType {
    None,
    Degradation,
    Loss,
};

inline constexpr std::string_view toString(const EnumConfigKey key) noexcept
{
    using namespace std::literals;

    switch (key) {
    case EnumConfigKey::SoulShrinkingTechnique:
        return "soulShrinkingTechnique"sv;
    case EnumConfigKey::SoulTrapLevelingType:
        return "soulTrapLevelingType"sv;
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
    fn(EnumConfigKey::SoulTrapLevelingType,
       static_cast<float>(SoulTrapLevelingType::None));
}

inline void forEachEnumConfigKey(const std::function<void(EnumConfigKey)>& fn)
{
    fn(EnumConfigKey::SoulShrinkingTechnique);
    fn(EnumConfigKey::SoulTrapLevelingType);
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

inline constexpr std::string_view
    toString(const SoulTrapLevelingType key) noexcept
{
    using namespace std::literals;

    switch (key) {
    case SoulTrapLevelingType::None:
        return "none"sv;
    case SoulTrapLevelingType::Degradation:
        return "degradation"sv;
    case SoulTrapLevelingType::Loss:
        return "loss"sv;
    }

    return ""sv;
}

inline constexpr std::string_view
    toString(const EnumConfigUnderlyingType value, const EnumConfigKey type)
{
    using namespace std::literals;

    switch (type) {
    case EnumConfigKey::SoulShrinkingTechnique:
        return toString(static_cast<SoulShrinkingTechnique>(value));
    case EnumConfigKey::SoulTrapLevelingType:
        return toString(static_cast<SoulShrinkingTechnique>(value));
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
struct EnumConfigKeyTypeMap<EnumConfigKey::SoulTrapLevelingType> {
    using type = SoulTrapLevelingType;

    type operator()(const float value) noexcept
    {
        if (value == static_cast<float>(type::Degradation)) {
            return type::Degradation;
        }

        if (value == static_cast<float>(type::Loss)) {
            return type::Loss;
        }

        return type::None;
    }
};

template <>
struct fmt::formatter<EnumConfigKey> {
    constexpr auto parse(fmt::format_parse_context& ctx)
        -> decltype(ctx.begin())
    {
        // [ctx.begin(), ctx.end()) is a character range that contains a part of
        // the format string starting from the format specifications to be
        // parsed, e.g. in
        //
        //   fmt::format("{:f} - point of interest", point(1, 2));
        //
        // the range will contain "f} - point of interest". The formatter should
        // parse specifiers until '}' or the end of the range.

        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin();

        // Check if reached the end of the range:
        if (it != ctx.end() && *it != '}') {
            throw fmt::format_error("invalid format");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const EnumConfigKey key, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), fmt::runtime(toString(key)));
    }
};

template <>
struct fmt::formatter<SoulShrinkingTechnique> {
    constexpr auto parse(fmt::format_parse_context& ctx)
        -> decltype(ctx.begin())
    {
        // [ctx.begin(), ctx.end()) is a character range that contains a part of
        // the format string starting from the format specifications to be
        // parsed, e.g. in
        //
        //   fmt::format("{:f} - point of interest", point(1, 2));
        //
        // the range will contain "f} - point of interest". The formatter should
        // parse specifiers until '}' or the end of the range.

        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin();

        // Check if reached the end of the range:
        if (it != ctx.end() && *it != '}') {
            throw fmt::format_error("invalid format");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const SoulShrinkingTechnique key, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), fmt::runtime(toString(key)));
    }
};

template <>
struct fmt::formatter<SoulTrapLevelingType> {
    constexpr auto parse(fmt::format_parse_context& ctx)
        -> decltype(ctx.begin())
    {
        // [ctx.begin(), ctx.end()) is a character range that contains a part of
        // the format string starting from the format specifications to be
        // parsed, e.g. in
        //
        //   fmt::format("{:f} - point of interest", point(1, 2));
        //
        // the range will contain "f} - point of interest". The formatter should
        // parse specifiers until '}' or the end of the range.

        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin();

        // Check if reached the end of the range:
        if (it != ctx.end() && *it != '}') {
            throw fmt::format_error("invalid format");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const SoulTrapLevelingType key, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), fmt::runtime(toString(key)));
    }
};
