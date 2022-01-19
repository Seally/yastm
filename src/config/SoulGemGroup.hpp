#pragma once

#include <exception>
#include <memory>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <toml++/toml_table.h>

#include <RE/S/SoulLevels.h>

#include "../utilities/stringutils.hpp"
#include "FormId.hpp"
#include "LoadPriority.hpp"
#include "SoulSize.hpp"

namespace RE {
    class TESDataHandler;
    class TESSoulGem;
} // end namespace RE

class SoulGemGroup {
public:
    using IdType = std::string;
    using MemberList = std::vector<FormId>;

private:
    IdType id_;
    bool isReusable_;
    SoulGemCapacity capacity_;
    LoadPriority priority_;
    MemberList members_;

public:
    explicit SoulGemGroup(const toml::table& table);

    [[nodiscard]] const IdType& id() const noexcept { return id_; }
    [[nodiscard]] bool isReusable() const noexcept { return isReusable_; }

    /**
     * @brief Returns the soul capacity of the soul gems in this group. Note
     * that this should never return SoulGemCapacity::Dual since we don't
     * support explicitly setting that value in the configuration files.
     */
    [[nodiscard]] SoulGemCapacity capacity() const noexcept { return capacity_; }
    /**
     * @brief Returns the "effective" soul gem capacity, used to match against
     * the values reported by the game soul gem forms.
     */
    [[nodiscard]] RE::SOUL_LEVEL effectiveCapacity() const
    {
        return toSoulLevel(capacity());
    }

    [[nodiscard]] LoadPriority rawPriority() const noexcept { return priority_; }
    [[nodiscard]] LoadPriority priority() const noexcept
    {
        if (rawPriority() == LoadPriority::Auto) {
            return isReusable() ? LoadPriority::High : LoadPriority::Normal;
        }

        return rawPriority();
    }

    [[nodiscard]] const MemberList& members() const noexcept { return members_; }
    [[nodiscard]] const FormId& emptyMember() const
    {
        return members().front();
    }
    [[nodiscard]] const FormId& filledMember() const
    {
        return members().back();
    }
};

class SoulGemGroupError : public std::runtime_error {
public:
    explicit SoulGemGroupError(const std::string& message)
        : std::runtime_error(message)
    {}
    explicit SoulGemGroupError(const char* message)
        : std::runtime_error(message)
    {}
};

template <>
struct fmt::formatter<SoulGemGroup> {
private:
    enum class Capitalization {
        AllLower,
        FirstUpper,
    };

    bool showReusability_ = false;
    bool showCapacity_ = false;
    bool showPriority_ = false;
    Capitalization capitalization_ = Capitalization::AllLower;

public:
    // Presentation format (in case of conflict, last format character wins):
    //
    // Capitalization:
    //
    //     'L': All lowercase (default).
    //     'u': Capitalize first letter.
    //
    // Capacity:
    //
    //     'c': Show capacity string ("black"/"petty"/"common"/etc.)
    //
    // Reusability:
    //
    //     'r': Show reusability string ("reusable"/"non-reusable")
    //
    // Priority:
    //     'p': Show priority and raw priority values.
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
        auto it = ctx.begin();

        for (; it != ctx.end() && *it != '}'; ++it) {
            switch (*it) {
            case 'L':
                capitalization_ = Capitalization::AllLower;
                break;
            case 'u':
                capitalization_ = Capitalization::FirstUpper;
                break;
            case 'r':
                showReusability_ = true;
                break;
            case 'c':
                showCapacity_ = true;
                break;
            case 'p':
                showPriority_ = true;
                break;
            default:
                throw format_error("invalid format");
            }
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const SoulGemGroup& group, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        // ctx.out() is an output iterator to write to.
        std::string formatString;

        if (showReusability_) {
            formatString.append(
                group.isReusable() ? "reusable " : "non-reusable ");
        }

        if (showCapacity_) {
            formatString.append(
                fmt::format(FMT_STRING("{} "), toString(group.capacity())));
        }

        formatString.append("soul gem group \"{}\"");

        if (showPriority_) {
            formatString.append(fmt::format(
                FMT_STRING(" (priority={}, rawPriority={})"),
                group.priority(),
                group.rawPriority()));
        }

        if (capitalization_ == Capitalization::FirstUpper) {
            capitalizeFirstChar(formatString, formatString);
        }

        return fmt::format_to(ctx.out(), formatString, group.id());
    }
};
