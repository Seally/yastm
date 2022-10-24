#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include <RE/B/BGSDefaultObjectManager.h>
#include <RE/B/BGSKeyword.h>
#include <RE/T/TESObjectREFR.h>
#include <RE/T/TESSoulGem.h>

#if defined(SKYRIM_VERSION_AE2)
#   include "native.hpp" 
#endif

namespace RE {
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

#if !defined(SKYRIM_VERSION_AE2)
/**
 * @brief Creates a new ExtraDataList, copying some properties from the
 * original.
 *
 * WARNING: The returned ExtraDataList is constructed on the heap within this
 * function and MUST be deleted or managed manually otherwise you WILL have
 * a memory leak.
 */
[[nodiscard]] inline std::unique_ptr<RE::ExtraDataList>
    createExtraDataListFromOriginal(RE::ExtraDataList* const originalExtraList)
{
    std::unique_ptr<RE::ExtraDataList> newExtraList;

    if (originalExtraList != nullptr) {
        // Inherit ownership.
        if (const auto owner = originalExtraList->GetOwner(); owner) {
            newExtraList.reset(new RE::ExtraDataList());
            newExtraList->SetOwner(owner);
        }
    }

    return newExtraList;
}
#else

/**
 * @brief Creates a new ExtraDataList, copying some properties from the
 * original.
 *
 * WARNING: The returned ExtraDataList is constructed on the heap within this
 * function and MUST be deleted or managed manually otherwise you WILL have
 * a memory leak.
 */
[[nodiscard]] inline RE::ExtraDataList*
    createExtraDataListFromOriginal(RE::ExtraDataList* const originalExtraList)
{
    RE::ExtraDataList* newExtraList = nullptr;

    if (originalExtraList != nullptr) {
        // Inherit ownership.
        if (const auto owner = originalExtraList->GetOwner(); owner) {
            newExtraList = native::BSExtraDataList::constructor();
            newExtraList->SetOwner(owner);
        }
    }

    return newExtraList;
}
#endif
