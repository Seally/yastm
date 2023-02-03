#pragma once

#include <string_view>
#include <type_traits>

#include <fmt/format.h>

#include <RE/B/BGSDefaultObjectManager.h>
#include <RE/S/SoulLevels.h>
#include <RE/T/TESSoulGem.h>

#include "../config/SoulSize.hpp"
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
private:
    enum class OutputFormat {
        Short,
        IdOnly,
        Full,
    };

    OutputFormat outputFormat_ = OutputFormat::Short;

public:
    // Presentation format (in case of conflict, last format character wins):
    //
    // Output format:
    //
    //     <none>:        Prints the display name, contained soul size, and form
    //                    ID.
    //                    e.g. [ID:0002E4E2] Petty Soul Gem (none)
    //     'i' (id-only): Prints only the form ID.
    //                    e.g. [ID:0002E4E2]
    //     'f' (full):    Prints everything.
    //                    e.g. [ID:0002E4E2] Petty Soul Gem (capacity=1, containedSoulSize=0, canHoldBlackSoul=false, reusable=false)
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
            case 'i':
                outputFormat_ = OutputFormat::IdOnly;
                break;
            case 'f':
                outputFormat_ = OutputFormat::Full;
                break;
            default:
                throw format_error("invalid format");
            }
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const RE::TESSoulGem& soulGemForm, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        using namespace std::literals;

        switch (outputFormat_) {
        case OutputFormat::IdOnly:
            return fmt::format_to(
                ctx.out(),
                FMT_STRING("[ID:{:08X}]"sv),
                soulGemForm.GetFormID());
        case OutputFormat::Full:
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
        default:
            return fmt::format_to(
                ctx.out(),
                FMT_STRING("[ID:{:08X}] {} ({})"sv),
                soulGemForm.GetFormID(),
                soulGemForm.GetName(),
                toString(soulGemForm.GetContainedSoul()));
        }
    }
};
