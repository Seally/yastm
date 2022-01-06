#pragma once

#include <compare>

#include <fmt/format.h>

#include <RE/A/Actor.h>

#include "../config/SoulSize.hpp"

class Victim {
    RE::Actor* actor_;
    SoulSize soulSize_;
    bool isSplit_;

public:
    /**
     * @brief Constructs a primary victim. The soul size automatically
     * calculated from the actor's properties.
     */
    explicit Victim(RE::Actor* actor);
    /**
     * @brief Constructs a victim with no associated actor. This constructor is
     * used for displaced souls.
     */
    explicit Victim(SoulSize soulSize);
    /**
     * @brief Constructs a victim with a custom soul size. This
     * constructor is used for split souls (the split flag is set
     * automatically).
     */
    explicit Victim(RE::Actor* actor, SoulSize soulSize);

    RE::Actor* actor() const { return actor_; }
    SoulSize soulSize() const { return soulSize_; }

    /**
     * @brief Primary souls are souls that we're currently capturing. These
     * souls are associated with an actor.
     *
     * Note that split souls may or may not be a primary soul depending on
     * whether the original soul was a displaced soul or the one we're soul
     * trapping.
     */
    bool isPrimarySoul() const { return actor() != nullptr; }
    /**
     * @brief Secondary souls are souls displaced from an existing soul gem.
     * These souls have no actor associated with them.
     */
    bool isSecondarySoul() const { return actor() == nullptr; }
    bool isSplitSoul() const { return isSplit_; }
};

inline auto operator<=>(const Victim& lhs, const Victim& rhs)
{
    return lhs.soulSize() <=> rhs.soulSize();
}

template <>
struct fmt::formatter<Victim> {
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
        auto it = ctx.begin(), end = ctx.end();

        // Check if reached the end of the range:
        if (it != end && *it != '}') {
            throw format_error("invalid format");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const Victim& victim, FormatContext& ctx) -> decltype(ctx.out())
    {
        using namespace std::literals;

        const RE::Actor* const actor = victim.actor();

        return format_to(
            ctx.out(),
            FMT_STRING("(soulSize={}, actor={}, isSplitSoul={})"sv),
            victim.soulSize(),
            actor != nullptr ? actor->GetName() : "null"sv,
            victim.isSplitSoul());
    }
};
