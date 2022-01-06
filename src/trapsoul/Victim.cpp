#include "Victim.hpp"

#include <cassert>

#include "../utilities/native.hpp"

SoulSize getActorSoulSize_(RE::Actor* const actor)
{
    assert(actor != nullptr);

    if (native::isActorNPC(actor)) {
        return SoulSize::Black;
    }

    return toSoulSize(native::getRemainingSoulLevel(actor));
}

Victim::Victim(RE::Actor* const actor)
    : actor_(actor)
    , soulSize_(getActorSoulSize_(actor))
    , isSplit_(false)
{}

Victim::Victim(const SoulSize soulSize)
    : actor_(nullptr)
    , soulSize_(soulSize)
    , isSplit_(false)
{}

Victim::Victim(RE::Actor* const actor, const SoulSize soulSize)
    : actor_(actor)
    , soulSize_(soulSize)
    , isSplit_(true)
{}
