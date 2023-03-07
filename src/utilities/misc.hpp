#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include <cassert>

#include <RE/B/BGSDefaultObjectManager.h>
#include <RE/B/BGSKeyword.h>
#include <RE/T/TESObjectREFR.h>
#include <RE/T/TESSoulGem.h>

#include "native.hpp"
#include "SoulSize.hpp"
#include "formatters/TESForm.hpp"

namespace RE {
    class Actor;
    class BGSKeyword;
    class TESBoundObject;
} // namespace RE

using UnorderedInventoryItemMap = std::unordered_map<
    RE::TESBoundObject*,
    std::pair<
        RE::TESObjectREFR::Count,
        std::unique_ptr<RE::InventoryEntryData>>>;

/**
 * @brief Like RE::TESObjectREFR::GetInventory(filter), but returns an
 * std::unordered_map instead of the awfully slow std::map (we don't need
 * pointers sorted). 
 */
[[nodiscard]] UnorderedInventoryItemMap getInventoryFor(
    RE::TESObjectREFR* objectRef,
    std::function<bool(RE::TESBoundObject&)> filter);

[[nodiscard]] RE::BGSKeyword* getReusableSoulGemKeyword();

[[nodiscard]] inline bool
    canHoldBlackSoul(const RE::TESSoulGem* const soulGemForm)
{
    return soulGemForm->GetFormFlags() &
           RE::TESSoulGem::RecordFlags::kCanHoldNPCSoul;
}

/**
 * @brief Creates a new ExtraDataList, copying some properties from the
 * original.
 */
[[nodiscard]] inline std::unique_ptr<RE::ExtraDataList>
    createExtraDataListFromOriginal(RE::ExtraDataList* const originalExtraList)
{
    std::unique_ptr<RE::ExtraDataList> newExtraList;

    if (originalExtraList != nullptr) {
        LOG_TRACE("Checking if we need to copy ownership...");

        if (const auto owner = originalExtraList->GetOwner(); owner) {
            LOG_TRACE("Owner found.");
            newExtraList.reset(new RE::ExtraDataList());
            LOG_TRACE_FMT("Copying owner: {}", *owner);
            newExtraList->SetOwner(owner);
        } else {
            LOG_TRACE("No owner exists. No need to copy extra data.");
        }
    }

    return newExtraList;
}

inline SoulSize getActorSoulSize(RE::Actor* const actor)
{
    assert(actor != nullptr);

    if (native::isActorNPC(actor)) {
        return SoulSize::Black;
    }

    return toSoulSize(native::getRemainingSoulLevel(actor));
}
