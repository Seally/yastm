#pragma once

#include <compare>

#include <cassert>

#include <fmt/format.h>

#include <RE/A/Actor.h>

#include "../SoulSize.hpp"
#include "../utilities/misc.hpp"
#include "../utilities/native.hpp"

class Victim {
    RE::Actor* actor_;
    SoulSize soulSize_;
    bool isSplit_;

public:
    /**
     * @brief Constructs a primary victim. The soul size calculated from the
     * actor's properties and the maximum soul size.
     */
    explicit Victim(RE::Actor* actor);
    /**
     * @brief Constructs a victim with no associated actor. This constructor is
     * used for displaced souls.
     */
    explicit Victim(SoulSize soulSize) noexcept
        : actor_(nullptr)
        , soulSize_(soulSize)
        , isSplit_(false)
    {}
    /**
     * @brief Constructs a victim with a custom soul size.
     */
    explicit Victim(RE::Actor* actor, SoulSize soulSize, bool isSplit) noexcept
        : actor_(actor)
        , soulSize_(soulSize)
        , isSplit_(isSplit)
    {}

    RE::Actor* actor() const noexcept { return actor_; }
    SoulSize soulSize() const noexcept { return soulSize_; }

    /**
     * @brief Primary souls are souls that we're currently capturing. These
     * souls are associated with an actor.
     *
     * Note that split souls may or may not be a primary soul depending on
     * whether the original soul was a displaced soul or the one we're soul
     * trapping.
     */
    bool isPrimarySoul() const noexcept { return actor() != nullptr; }
    /**
     * @brief Secondary souls are souls displaced from an existing soul gem.
     * These souls have no actor associated with them.
     */
    bool isSecondarySoul() const noexcept { return actor() == nullptr; }
    bool isSplitSoul() const noexcept { return isSplit_; }
};

inline Victim::Victim(RE::Actor* const actor)
    : actor_(actor)
    , soulSize_(getActorSoulSize(actor))
    , isSplit_(false)
{}

inline auto operator<=>(const Victim& lhs, const Victim& rhs) noexcept
{
    return lhs.soulSize() <=> rhs.soulSize();
}

template <>
struct fmt::formatter<Victim> {
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
    auto format(const Victim& victim, FormatContext& ctx) -> decltype(ctx.out())
    {
        using namespace std::literals;

        const RE::Actor* const actor = victim.actor();

        return fmt::format_to(
            ctx.out(),
            FMT_STRING("(soulSize={}, actor={}, isSplitSoul={})"sv),
            victim.soulSize(),
            actor != nullptr ? actor->GetName() : "null"sv,
            victim.isSplitSoul());
    }
};
