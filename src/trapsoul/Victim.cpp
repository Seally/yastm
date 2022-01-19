#include "Victim.hpp"

#include <cassert>

#include "../utilities/native.hpp"

namespace {
    SoulSize getActorSoulSize_(RE::Actor* const actor)
    {
        assert(actor != nullptr);

        if (native::isActorNPC(actor)) {
            return SoulSize::Black;
        }

        return toSoulSize(native::getRemainingSoulLevel(actor));
    }
} // namespace

Victim::Victim(RE::Actor* const actor)
    : actor_(actor)
    , soulSize_(getActorSoulSize_(actor))
    , isSplit_(false)
{}
