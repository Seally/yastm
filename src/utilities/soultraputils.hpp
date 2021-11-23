#pragma once

#include <REL/Relocation.h>
#include <RE/B/BGSDefaultObjectManager.h>
#include <RE/B/BGSKeyword.h>
#include <RE/S/SoulLevels.h>
#include <RE/T/TESSoulGem.h>

#include "../global.hpp"
#include "../offsets.hpp"
#include "config/SoulSize.hpp"

RE::BGSKeyword* getReusableSoulGemKeyword();

inline bool canHoldBlackSoul(const RE::TESSoulGem* const soulGemForm)
{
    return soulGemForm->GetFormFlags() &
           RE::TESSoulGem::RecordFlags::kCanHoldNPCSoul;
}

inline bool isActorNPC(RE::Actor* const actor)
{
    using func_t = decltype(isActorNPC);
    REL::Relocation<func_t> func(re::soultraputils::Actor::IsActorNPC);
    return func(actor);
}

inline RE::SOUL_LEVEL getRemainingSoulLevel(RE::Actor* const actor)
{
    actor->GetLevel();
    using func_t = decltype(getRemainingSoulLevel);
    REL::Relocation<func_t> func(
        re::soultraputils::Actor::GetRemainingSoulLevel);
    return func(actor);
}

/**
 * @brief Returns the remaining "raw" soul size of the actor.
 *
 * The raw soul size is the actual capacity of the soul. They're mapped to
 * the enumerated soul sizes as follows:
 *
 * - None = 0
 * - Petty = 250
 * - Lesser = 500
 * - Common = 1000
 * - Greater = 2000
 * - Grand = 3000
 *
 * @returns 0 if the actor has already been soul trapped, otherwise returns
 * their raw soul size.
 */
inline SoulLevelValue getRemainingSoulLevelValue(RE::Actor* const actor)
{
    using func_t = decltype(getRemainingSoulLevelValue);
    REL::Relocation<func_t> func(
        re::soultraputils::Actor::GetRemainingSoulLevelValue);
    return func(actor);
}
