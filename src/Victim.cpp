#include "Victim.hpp"

#include <cassert>

namespace native {
    RE::SOUL_LEVEL GetSoulSize(RE::Actor* const actor)
    {

        using func_t = decltype(GetSoulSize);
        // SkyrimSE.exe + 0x6348A0 (v1.5.97.0)
        REL::Relocation<func_t> func{REL::ID{37862}};
        return func(actor);
    }

    bool IsActorNPC(RE::Actor* const actor)
    {
        using func_t = decltype(IsActorNPC);
        // SkyrimSE.exe + 0x606850 (v1.5.97.0)
        REL::Relocation<func_t> func{REL::ID{36889}};
        return func(actor);
    }
} // namespace native

SoulSize _getActorSoulSize(RE::Actor* const actor)
{
    assert(actor != nullptr);

    if (native::IsActorNPC(actor)) {
        return SoulSize::Black;
    }

    return static_cast<SoulSize>(native::GetSoulSize(actor));
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
