#pragma once

#include <fmt/format.h>

#include <RE/T/TESSoulGem.h>
#include <RE/B/BGSDefaultObjectManager.h>
#include "../utilities/soultraputilities.hpp"

template <>
struct fmt::formatter<RE::TESSoulGem> {
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
    auto format(const RE::TESSoulGem& soulGemForm, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        using namespace std::literals;

        return fmt::format_to(
            ctx.out(),
            FMT_STRING(
                "[ID:{:08x}] {} (capacity={}, containedSoulSize={}, canHoldBlackSoul={}, reusable={})"sv),
            soulGemForm.GetFormID(),
            soulGemForm.GetName(),
            soulGemForm.GetMaximumCapacity(),
            soulGemForm.GetContainedSoul(),
            canHoldBlackSoul(&soulGemForm),
            soulGemForm.HasKeyword(getReusableSoulGemKeyword()));
    }
};
