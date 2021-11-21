#pragma once

#include <REL/Relocation.h>
#include <RE/B/BGSDefaultObjectManager.h>
#include <RE/B/BGSKeyword.h>
#include <RE/S/SoulLevels.h>
#include <RE/T/TESSoulGem.h>

#include "../global.hpp"
#include "config/SoulSize.hpp"

RE::BGSKeyword* getReusableSoulGemKeyword();

inline bool canHoldBlackSoul(const RE::TESSoulGem* const soulGemForm)
{
    return soulGemForm->GetFormFlags() &
           RE::TESSoulGem::RecordFlags::kCanHoldNPCSoul;
}

// TODO: Possibly related to/same as Actor::CalculateCachedOwnerIsNPC()
inline bool isActorNPC(RE::Actor* const actor)
{
    using func_t = decltype(isActorNPC);
    // SkyrimSE.exe + 0x606850 [1.5.97.0]  [ADDRLIB:36889]
    // SkyrimSE.exe + 0x62de60 [1.6.318.0] [ADDRLIB:37913]
    REL::Relocation<func_t> func(REL::ID(37913));
    return func(actor);
}

//inline RE::SOUL_LEVEL
//    calculateSoulLevel(const std::uint32_t actorLevel, const bool isNPC)
//{
//    using func_t = decltype(calculateSoulLevel);
//    // SkyrimSE.exe + 0x3c1740 [1.5.97.0]  [ADDRLIB:25933]
//    // SkyrimSE.exe + 0x3d91a0 [1.6.318.0] [ADDRLIB:26520]
//    REL::Relocation<func_t> func(REL::ID(26520));
//    return func(actorLevel, isNPC);
//}
//
//inline RE::SOUL_LEVEL getSoulLevel(RE::Actor* const actor) {
//    return calculateSoulLevel(actor->GetLevel(), isActorNPC(actor));
//}

inline RE::SOUL_LEVEL getRemainingSoulLevel(RE::Actor* const actor)
{
    actor->GetLevel();
    using func_t = decltype(getRemainingSoulLevel);
    // SkyrimSE.exe + 0x6348a0 [1.5.97.0]  [ADDRLIB:37862]
    // SkyrimSE.exe + 0x65a970 [1.6.318.0] [ADDRLIB:38817]
    REL::Relocation<func_t> func(REL::ID(38817));
    return func(actor);
}

///**
// * Converts RE::SOUL_LEVEL to a SoulLevelValue. 
// */
//inline SoulLevelValue toSoulLevelValue(const RE::SOUL_LEVEL soulLevel)
//{
//    using func_t = SoulLevelValue(RE::SOUL_LEVEL);
//    // SkyrimSE.exe + 0x237a90 [1.5.97.0]  [ADDRLIB:17753]
//    // SkyrimSE.exe + 0x247e20 [1.6.318.0] [ADDRLIB:18166]
//    REL::Relocation<func_t> func(REL::ID(18166));
//    return func(soulLevel);
//}

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
    // SkyrimSE.exe + 0x634830 [1.5.97.0]  [ADDRLIB:37861]
    // SkyrimSE.exe + 0x65a900 [1.6.318.0] [ADDRLIB:38816] (inlined but not removed in source).
    REL::Relocation<func_t> func(REL::ID(38816));
    return func(actor);
}
