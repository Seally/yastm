#pragma once

#include <fmt/format.h>

#include <RE/A/Actor.h>

#include "config/SoulSize.hpp"

class Victim {
    RE::Actor* _actor;
    SoulSize _soulSize;
    bool _isSplit;

public:
    /**
     * @brief Constructs a victim object with the soul size automatically
     * calculated from the actor's properties. This constructor is used for the
     * initial victim. 
     */
    explicit Victim(RE::Actor* actor);
    /**
     * @brief Constructs a victim object with only the soul size. This
     * constructor is used for displaced souls.
     */
    explicit Victim(SoulSize soulSize);
    /**
     * @brief Construct a victim object with both a (possibly null) actor and a
     * specified soul size. This constructor is used for split souls (the split
     * flag is set automatically).
     */
    explicit Victim(RE::Actor* actor, SoulSize soulSize);

    RE::Actor* actor() const { return _actor; }
    SoulSize soulSize() const { return _soulSize; }

    bool isPrimarySoul() const { return actor() != nullptr; }
    bool isSecondarySoul() const { return actor() == nullptr; }
    bool isSplitSoul() const { return _isSplit; }
};

inline bool operator<(const Victim& lhs, const Victim& rhs)
{
    return lhs.soulSize() < rhs.soulSize();
}

inline bool operator<=(const Victim& lhs, const Victim& rhs)
{
    return lhs.soulSize() <= rhs.soulSize();
}

inline bool operator>(const Victim& lhs, const Victim& rhs)
{
    return lhs.soulSize() > rhs.soulSize();
}

inline bool operator>=(const Victim& lhs, const Victim& rhs)
{
    return lhs.soulSize() >= rhs.soulSize();
}

template <>
struct fmt::formatter<Victim> {
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
