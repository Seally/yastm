#pragma once

#include <exception>
#include <unordered_map>

#include "SoulGemGroup.hpp"
#include "SoulSize.hpp"

#include "../utilities/stringutils.hpp"

namespace RE {
    class TESDataHandler;
    class TESSoulGem;
} // end namespace RE

class ConcreteSoulGemGroup {
public:
    using IdType = SoulGemGroup::IdType;

private:
    using FormMap = std::unordered_map<SoulSize, RE::TESSoulGem*>;

    IdType _id;
    SoulGemCapacity _capacity;

    FormMap _forms;

    void _initializeFromPrimaryBasis(
        const SoulGemGroup& sourceGroup,
        RE::TESDataHandler* dataHandler);
    void _initializeFromSecondaryBasis(
        const ConcreteSoulGemGroup& blackSoulGemGroup);

public:
    explicit ConcreteSoulGemGroup(
        const SoulGemGroup& sourceGroup,
        RE::TESDataHandler* dataHandler);
    explicit ConcreteSoulGemGroup(
        const SoulGemGroup& whiteGrandSoulGemGroup,
        const ConcreteSoulGemGroup& blackSoulGemGroup,
        RE::TESDataHandler* dataHandler);

    [[nodiscard]] const IdType& id() const { return _id; }
    [[nodiscard]] SoulGemCapacity capacity() const { return _capacity; }

    [[nodiscard]] RE::TESSoulGem* at(const SoulSize containedSoulSize) const
    {
        const auto result = _forms.find(containedSoulSize);

        if (result != _forms.end()) {
            return result->second;
        }

        return nullptr;
    }

    [[nodiscard]] auto begin() const { return _forms.begin(); }
    [[nodiscard]] auto end() const { return _forms.end(); }
};

class ConcreteSoulGemGroupError : public std::runtime_error {
public:
    explicit ConcreteSoulGemGroupError(const std::string& message);
};

template <>
struct fmt::formatter<ConcreteSoulGemGroup> {
private:
    enum class Capitalization {
        AllLower,
        FirstUpper,
    };

    bool _showReusability = false;
    bool _showCapacity = false;
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
        auto it = ctx.begin();

        for (; it != ctx.end() && *it != '}'; ++it) {
            switch (*it) {
            case 'L':
                _capitalization = Capitalization::AllLower;
                break;
            case 'u':
                _capitalization = Capitalization::FirstUpper;
                break;
            case 'c':
                _showCapacity = true;
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
    auto format(const ConcreteSoulGemGroup& group, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        std::string formatString;

        if (_showCapacity) {
            formatString.append(
                fmt::format(FMT_STRING("{} "), toString(group.capacity())));
        }

        formatString.append("soul gem group \"{}\"");

        if (_capitalization == Capitalization::FirstUpper) {
            capitalizeFirstChar(formatString, formatString);
        }

        return fmt::format_to(ctx.out(), formatString, group.id());
    }
};
