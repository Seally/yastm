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
    IdType _id;
    bool _isReusable;
    SoulGemCapacity _capacity;
    LoadPriority _priority;
    MemberList _members;

public:
    explicit SoulGemGroup(const toml::table& table);

    [[nodiscard]] const IdType& id() const { return _id; }
    [[nodiscard]] bool isReusable() const { return _isReusable; }

    /**
     * @brief Returns the soul capacity of the soul gems in this group. Note
     * that this should never return SoulGemCapacity::Dual since we don't
     * support explicitly setting that value in the configuration files.
     */
    [[nodiscard]] SoulGemCapacity capacity() const { return _capacity; }
    /**
     * @brief Returns the "effective" soul gem capacity, used to match against
     * the values reported by the game soul gem forms.
     */
    [[nodiscard]] RE::SOUL_LEVEL effectiveCapacity() const
    {
        return toSoulLevel(capacity());
    }

    [[nodiscard]] LoadPriority rawPriority() const { return _priority; }
    [[nodiscard]] LoadPriority priority() const
    {
        if (rawPriority() == LoadPriority::Auto) {
            return isReusable() ? LoadPriority::High : LoadPriority::Normal;
        }

        return rawPriority();
    }

    [[nodiscard]] const MemberList& members() const { return _members; }
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

    bool _showReusability = false;
    bool _showCapacity = false;
    bool _showPriority = false;
    Capitalization _capitalization = Capitalization::AllLower;

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
                _capitalization = Capitalization::AllLower;
                break;
            case 'u':
                _capitalization = Capitalization::FirstUpper;
                break;
            case 'r':
                _showReusability = true;
                break;
            case 'c':
                _showCapacity = true;
                break;
            case 'p':
                _showPriority = true;
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

        if (_showReusability) {
            formatString.append(
                group.isReusable() ? "reusable " : "non-reusable ");
        }

        if (_showCapacity) {
            formatString.append(
                fmt::format(FMT_STRING("{} "), toString(group.capacity())));
        }

        formatString.append("soul gem group \"{}\"");

        if (_showPriority) {
            formatString.append(fmt::format(
                FMT_STRING(" (priority={}, rawPriority={})"),
                group.priority(),
                group.rawPriority()));
        }

        if (_capitalization == Capitalization::FirstUpper) {
            capitalizeFirstChar(formatString, formatString);
        }

        return fmt::format_to(ctx.out(), formatString, group.id());
    }
};
