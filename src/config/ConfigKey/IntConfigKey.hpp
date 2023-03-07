#pragma once

#include <functional>
#include <string>

#include <fmt/format.h>

enum class IntConfigKey {
    SoulTrapThresholdPetty,
    SoulTrapThresholdLesser,
    SoulTrapThresholdCommon,
    SoulTrapThresholdGreater,
    SoulTrapThresholdGrand,
    SoulTrapThresholdBlack,
    Count,
};

inline constexpr std::string_view toString(const IntConfigKey key) noexcept
{
    using namespace std::literals;

    switch (key) {
    case IntConfigKey::SoulTrapThresholdPetty:
        return "soulTrapThresholdPetty"sv;
    case IntConfigKey::SoulTrapThresholdLesser:
        return "soulTrapThresholdLesser"sv;
    case IntConfigKey::SoulTrapThresholdCommon:
        return "soulTrapThresholdCommon"sv;
    case IntConfigKey::SoulTrapThresholdGreater:
        return "soulTrapThresholdGreater"sv;
    case IntConfigKey::SoulTrapThresholdGrand:
        return "soulTrapThresholdGrand"sv;
    case IntConfigKey::SoulTrapThresholdBlack:
        return "soulTrapThresholdBlack"sv;
    case IntConfigKey::Count:
        return "<count>"sv;
    }

    return "<invalid IntConfigKey>"sv;
}

inline void
    forEachIntConfigKey(const std::function<void(IntConfigKey, float)>& fn)
{
    fn(IntConfigKey::SoulTrapThresholdPetty, static_cast<float>(1));
    fn(IntConfigKey::SoulTrapThresholdLesser, static_cast<float>(20));
    fn(IntConfigKey::SoulTrapThresholdCommon, static_cast<float>(25));
    fn(IntConfigKey::SoulTrapThresholdGreater, static_cast<float>(32));
    fn(IntConfigKey::SoulTrapThresholdGrand, static_cast<float>(40));
    fn(IntConfigKey::SoulTrapThresholdBlack, static_cast<float>(50));
}

inline void forEachIntConfigKey(const std::function<void(IntConfigKey)>& fn)
{
    fn(IntConfigKey::SoulTrapThresholdPetty);
    fn(IntConfigKey::SoulTrapThresholdLesser);
    fn(IntConfigKey::SoulTrapThresholdCommon);
    fn(IntConfigKey::SoulTrapThresholdGreater);
    fn(IntConfigKey::SoulTrapThresholdGrand);
    fn(IntConfigKey::SoulTrapThresholdBlack);
}

template <>
struct fmt::formatter<IntConfigKey> {
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
    auto format(const IntConfigKey key, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), fmt::runtime(toString(key)));
    }
};
