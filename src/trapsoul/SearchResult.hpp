#pragma once

#include <RE/T/TESObjectREFR.h>

#include "../config/SoulGemMap.hpp"

namespace RE {
    class InventoryEntryData;
    class TESSoulGem;
} // namespace RE

class SearchResult {
    const SoulGemMap::Iterator it_;
    const RE::TESObjectREFR::Count itemCount_;
    RE::InventoryEntryData* const entryData_;

public:
    explicit SearchResult(
        const SoulGemMap::Iterator it,
        const RE::TESObjectREFR::Count itemCount,
        RE::InventoryEntryData* const entryData)
        : it_(it)
        , itemCount_(itemCount)
        , entryData_(entryData)
    {}

    RE::TESObjectREFR::Count itemCount() const noexcept { return itemCount_; }
    RE::InventoryEntryData* entryData() const noexcept { return entryData_; }

    const ConcreteSoulGemGroup& group() const { return it_.group(); }
    const SoulSize containedSoulSize() const noexcept { return it_.containedSoulSize(); }

    RE::TESSoulGem* soulGem() const { return it_.get(); }
    RE::TESSoulGem* soulGemAt(const SoulSize containedSoulSize) const
    {
        return it_.group().at(containedSoulSize);
    }
};
