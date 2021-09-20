#include "Victim.hpp"

namespace Native {
    RE::SOUL_LEVEL GetSoulSize(RE::Actor* const actor) {
        using func_t = decltype(GetSoulSize);
        REL::Relocation<func_t> func{REL::ID{37862}}; // SkyrimSE.exe + 0x6348A0 (v1.5.97.0)
        return func(actor);
    }

    bool IsActorNPC(RE::Actor* const actor) {
        using func_t = decltype(IsActorNPC);
        REL::Relocation<func_t> func{REL::ID{36889}}; // SkyrimSE.exe + 0x606850 (v1.5.97.0)
        return func(actor);
    }
} // namespace Native

SoulSize _getActorSoulSize(RE::Actor* const actor) {
    if (Native::IsActorNPC(actor)) {
        return SoulSize::Black;
    }

    return static_cast<SoulSize>(Native::GetSoulSize(actor));
}

Victim::Victim(RE::Actor* const actor) : _actor{actor}, _soulSize{_getActorSoulSize(actor)} {}

Victim::Victim(const SoulSize soulSize) : _actor{nullptr}, _soulSize{soulSize} {}
