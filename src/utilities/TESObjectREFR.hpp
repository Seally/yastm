#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include <RE/T/TESObjectREFR.h>

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
UnorderedInventoryItemMap getInventoryFor(
    RE::TESObjectREFR* objectRef,
    std::function<bool(RE::TESBoundObject&)> filter);
