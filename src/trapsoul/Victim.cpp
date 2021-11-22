#include "Victim.hpp"

#include <cassert>

#include "../utilities/SoulTrapUtilities.hpp"

SoulSize _getActorSoulSize(RE::Actor* const actor)
{
    assert(actor != nullptr);

    if (isActorNPC(actor)) {
        return SoulSize::Black;
    }

    return toSoulSize(getRemainingSoulLevel(actor));
}

Victim::Victim(RE::Actor* const actor)
    : _actor{actor}
    , _soulSize{_getActorSoulSize(actor)}
    , _isSplit{false}
{}

Victim::Victim(const SoulSize soulSize)
    : _actor{nullptr}
    , _soulSize{soulSize}
    , _isSplit{false}
{}

Victim::Victim(RE::Actor* const actor, const SoulSize soulSize)
    : _actor{actor}
    , _soulSize{soulSize}
    , _isSplit{true}
{}
