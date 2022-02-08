#pragma once

#include <string_view>
#include <type_traits>

#include <fmt/format.h>

#include <RE/B/BGSDefaultObjectManager.h>
#include <RE/S/SoulLevels.h>
#include <RE/T/TESSoulGem.h>

#include "../utilities/misc.hpp"

template <>
struct fmt::formatter<RE::SOUL_LEVEL> :
    fmt::formatter<std::underlying_type_t<RE::SOUL_LEVEL>> {
    template <typename FormatContext>
    auto format(const RE::SOUL_LEVEL soulLevel, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        using EnumType = std::underlying_type_t<RE::SOUL_LEVEL>;

        return fmt::formatter<EnumType>::format(
            static_cast<EnumType>(soulLevel),
            ctx);
    }
};

template <>
struct fmt::formatter<RE::TESSoulGem> {
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

        // Check if reached the end of the range:
        if (it != ctx.end() && *it != '}') {
            throw format_error("invalid format");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const RE::TESSoulGem& soulGemForm, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        using namespace std::literals;

        return fmt::format_to(
            ctx.out(),
            FMT_STRING(
                "[ID:{:08X}] {} (capacity={}, containedSoulSize={}, canHoldBlackSoul={}, reusable={})"sv),
            soulGemForm.GetFormID(),
            soulGemForm.GetName(),
            soulGemForm.GetMaximumCapacity(),
            soulGemForm.GetContainedSoul(),
            canHoldBlackSoul(&soulGemForm),
            soulGemForm.HasKeyword(getReusableSoulGemKeyword()));
    }
};
