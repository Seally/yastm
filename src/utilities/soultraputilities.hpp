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

inline RE::SOUL_LEVEL getSoulSize(RE::Actor* const actor)
{
    using func_t = decltype(getSoulSize);
    // SkyrimSE.exe + 0x6348a0 [1.5.97.0]
    // SkyrimSE.exe + 0x65a970 [1.6.318.0]
#if defined(SKYRIM_VERSION_SE)
    REL::Relocation<func_t> func{REL::ID{37862}};
#elif defined(SKYRIM_VERSION_AE)
    REL::Relocation<func_t> func{REL::Offset{0x65a970}};
#endif
    return func(actor);
}

inline bool isActorNPC(RE::Actor* const actor)
{
    using func_t = decltype(isActorNPC);
    // SkyrimSE.exe + 0x606850 [1.5.97.0]
    // SkyrimSE.exe + 0x62de60 [1.6.318.0]
#if defined(SKYRIM_VERSION_SE)
    REL::Relocation<func_t> func{REL::ID{36889}};
#elif defined(SKYRIM_VERSION_AE)
    REL::Relocation<func_t> func{REL::Offset{0x62de60}};
#endif
    return func(actor);
}

inline RE::SOUL_LEVEL getRemainingSoulLevel(RE::Actor* const actor)
{
    using func_t = decltype(getRemainingSoulLevel);
    // SkyrimSE.exe + 0x6348a0 [1.5.97.0]
    // SkyrimSE.exe + 0x65a970 [1.6.318.0]
#if defined(SKYRIM_VERSION_SE)
    REL::Relocation<func_t> func{REL::ID{37862}};
#elif defined(SKYRIM_VERSION_AE)
    REL::Relocation<func_t> func{REL::Offset{0x65a970}};
#endif
    return func(actor);
}

inline RE::SOUL_LEVEL
    calculateSoulLevel(std::uint32_t actorLevel, const bool isNPC)
{
    using func_t = decltype(calculateSoulLevel);
    // SkyrimSE.exe + 0x3c1740 [1.5.97.0]
    // SkyrimSE.exe + 0x3d91a0 [1.6.318.0]
#if defined(SKYRIM_VERSION_SE)
    REL::Relocation<func_t> func{REL::ID{25933}};
#elif defined(SKYRIM_VERSION_AE)
    REL::Relocation<func_t> func{REL::Offset{0x3d91a0}};
#endif
    return func(actorLevel, isNPC);
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
inline SoulLevelValue getRemainingSoulLevelValue(const RE::SOUL_LEVEL soulLevel)
{
    using func_t = SoulLevelValue(RE::SOUL_LEVEL soulSize);
    // SkyrimSE.exe + 0x237a90 [1.5.97.0]
    // SkyrimSE.exe + 0x247e20 [1.6.318.0]
#if defined(SKYRIM_VERSION_SE)
    REL::Relocation<func_t> func{REL::ID{17753}};
#elif defined(SKYRIM_VERSION_AE)
    REL::Relocation<func_t> func{REL::Offset{0x247e20}};
#endif
    return func(soulLevel);
}

inline SoulLevelValue getRemainingSoulLevelValue(RE::Actor* const actor)
{
    using func_t = SoulLevelValue(RE::Actor * actor);
    // SkyrimSE.exe + 0x634830 [1.5.97.0]
    // SkyrimSE.exe + 0x65a900 [1.6.318.0] (inlined but not removed in source).
#if defined(SKYRIM_VERSION_SE)
    REL::Relocation<func_t> func{REL::ID{37861}};
#elif defined(SKYRIM_VERSION_AE)
    REL::Relocation<func_t> func{REL::Offset{0x65a900}};
#endif
    return func(actor);
}
