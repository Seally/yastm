#pragma once

#include <RE/T/TESObjectREFR.h>

#include "../config/SoulGemMap.hpp"

namespace RE {
    class InventoryEntryData;
    class TESSoulGem;
} // namespace RE

class SearchResult {
    const SoulGemMap::Iterator _it;
    const RE::TESObjectREFR::Count _itemCount;
    RE::InventoryEntryData* const _entryData;

public:
    explicit SearchResult(
        const SoulGemMap::Iterator it,
        const RE::TESObjectREFR::Count itemCount,
        RE::InventoryEntryData* const entryData)
        : _it(it)
        , _itemCount(itemCount)
        , _entryData(entryData)
    {}

    RE::TESObjectREFR::Count itemCount() const { return _itemCount; }
    RE::InventoryEntryData* entryData() const { return _entryData; }

    const ConcreteSoulGemGroup& group() const { return _it.group(); }
    const SoulSize containedSoulSize() const { return _it.containedSoulSize(); }

    RE::TESSoulGem* soulGem() const { return _it.get(); }
    RE::TESSoulGem* soulGemAt(const SoulSize containedSoulSize) const
    {
        return _it.group().at(containedSoulSize);
    }
};
