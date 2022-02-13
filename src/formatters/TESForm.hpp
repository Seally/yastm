#pragma once

#include <cassert>

#include <fmt/format.h>

#include <RE/T/TESDataHandler.h>
#include <RE/T/TESForm.h>

#include "../utilities/formidutils.hpp"

template <>
struct fmt::formatter<RE::TESForm> {
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
    auto format(const RE::TESForm& form, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        const auto dataHandler = RE::TESDataHandler::GetSingleton();

        assert(dataHandler != nullptr);

        const auto fileName = getModName(form.GetFormID(), dataHandler);

        if (fileName.has_value()) {
            return fmt::format_to(
                ctx.out(),
                FMT_STRING("[{:#8x}, {}]"),
                getLocalFormId(form.GetFormID()),
                *fileName);
        }

        return fmt::format_to(
            ctx.out(),
            FMT_STRING("[ID:{:08X}]"),
            form.GetFormID());
    }
};
