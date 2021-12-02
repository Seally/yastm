#pragma once

#include <REL/Relocation.h>
#include <RE/S/SoulLevels.h>

#include "../offsets.hpp"
#include "config/SoulSize.hpp"

namespace RE {
    class Actor;
    class ExtraDataList;
    class TESBoundObject;
} // namespace RE

namespace native {
    inline bool isActorNPC(RE::Actor* const actor)
    {
        using func_t = decltype(&isActorNPC);
        REL::Relocation<func_t> func(re::soultraputils::Actor::IsActorNPC);
        return func(actor);
    }

    inline RE::SOUL_LEVEL getRemainingSoulLevel(RE::Actor* const actor)
    {
        using func_t = decltype(&getRemainingSoulLevel);
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
        using func_t = decltype(&getRemainingSoulLevelValue);
        REL::Relocation<func_t> func(
            re::soultraputils::Actor::GetRemainingSoulLevelValue);
        return func(actor);
    }

    namespace BSExtraDataList {
        inline void SetSoul(
            RE::ExtraDataList* const dataList,
            const RE::SOUL_LEVEL soulLevel)
        {
            using func_t = decltype(&SetSoul);
            REL::Relocation<func_t> func(re::BSExtraDataList::SetSoul);
            func(dataList, soulLevel);
        }
    } // namespace BSExtraDataList

    inline void updateInventory(
        RE::TESObjectREFR* const ref,
        RE::TESBoundObject* const obj)
    {
        using func_t = decltype(&updateInventory);
        REL::Relocation<func_t> func(re::fix::chargeitem::updateInventory);
        func(ref, obj);
    }
} // namespace native
