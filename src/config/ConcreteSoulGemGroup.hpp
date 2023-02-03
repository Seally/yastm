#pragma once

#include <exception>
#include <unordered_map>

#include "SoulGemGroup.hpp"
#include "SoulSize.hpp"

#include "../global.hpp"
#include "../utilities/stringutils.hpp"

namespace RE {
    class TESDataHandler;
    class TESSoulGem;
} // end namespace RE

/**
 * @brief Represents a group of soul gem forms. This class differs from
 * SoulGemGroup in that the forms stored are already resolved to pointers to the
 * in-game forms.
 */
class ConcreteSoulGemGroup {
public:
    using IdType = SoulGemGroup::IdType;

private:
    using FormMap = std::unordered_map<SoulSize, RE::TESSoulGem*>;

    IdType id_;
    SoulGemCapacity capacity_;

    FormMap forms_;

    void initializeFromPrimaryBasis_(
        const SoulGemGroup& sourceGroup,
        RE::TESDataHandler* dataHandler);
    void initializeFromSecondaryBasis_(
        const ConcreteSoulGemGroup& blackSoulGemGroup);

public:
    /**
     * @brief Constructs a pure soul gem group.
     * 
     * @param[in] sourceGroup The soul gem group to store.
     * @param[in] dataHandler The data handler used to retrieve the in-game
     * forms.
     */
    explicit ConcreteSoulGemGroup(
        const SoulGemGroup& sourceGroup,
        RE::TESDataHandler* dataHandler);
    /**
     * @brief Constructs a dual soul gem group.
     *
     * @param[in] whiteGrandSoulGemGroup The grand soul gem group to use as the
     * primary basis for the group. This is the "real" group that is stored.
     * @param[in] blackSoulGemGroup A previously instantiated
     * ConcreteSoulGemGroup of black soul gems to use as the secondary basis for
     * the group. This is used to look up the black soul-containing soul gem
     * form.
     * @param[in] dataHandler The data handler used to retrieve the in-game
     * forms.
     */
    explicit ConcreteSoulGemGroup(
        const SoulGemGroup& whiteGrandSoulGemGroup,
        const ConcreteSoulGemGroup& blackSoulGemGroup,
        RE::TESDataHandler* dataHandler);

    [[nodiscard]] const IdType& id() const noexcept { return id_; }
    [[nodiscard]] SoulGemCapacity capacity() const noexcept
    {
        return capacity_;
    }

    RE::TESSoulGem* at(const SoulSize containedSoulSize) const
    {
        const auto result = forms_.find(containedSoulSize);

        if (result != forms_.end()) {
            return result->second;
        }

        return nullptr;
    }

    [[nodiscard]] auto begin() const noexcept { return forms_.begin(); }
    [[nodiscard]] auto end() const noexcept { return forms_.end(); }
};

class ConcreteSoulGemGroupError : public std::runtime_error {
public:
    explicit ConcreteSoulGemGroupError(const std::string& message)
        : std::runtime_error(message)
    {}
    explicit ConcreteSoulGemGroupError(const char* message)
        : std::runtime_error(message)
    {}
};

template <>
struct fmt::formatter<ConcreteSoulGemGroup> {
private:
    enum class Capitalization {
        AllLower,
        FirstUpper,
    };

    bool showReusability_ = false;
    bool showCapacity_ = false;
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

        for (; it != ctx.end() && *it != '}'; ++it) {
            switch (*it) {
            case 'L':
                capitalization_ = Capitalization::AllLower;
                break;
            case 'u':
                capitalization_ = Capitalization::FirstUpper;
                break;
            case 'c':
                showCapacity_ = true;
                break;
            default:
                throw fmt::format_error("invalid format");
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

        if (showCapacity_) {
            formatString.append(
                fmt::format(FMT_STRING("{} "), toString(group.capacity())));
        }

        formatString.append("soul gem group \"{}\"");

        if (capitalization_ == Capitalization::FirstUpper) {
            capitalizeFirstChar(formatString);
        }

        return fmt::format_to(
            ctx.out(),
            fmt::runtime(formatString),
            group.id());
    }
};
