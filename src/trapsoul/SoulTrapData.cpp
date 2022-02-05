#include "SoulTrapData.hpp"

#include <cassert>

void SoulTrapData::resetInventoryData_()
{
    std::size_t maxFilledSoulGemsCount = 0;

    // This should be a move.
    inventoryMap_ =
        getInventoryFor(caster_, [&](const RE::TESBoundObject& obj) {
            return obj.IsSoulGem();
        });

    // Counts the number of fully-filled soul gems.
    //
    // Note: This ignores the fact that we can still displace white
    // grand souls from black soul gems and vice versa.
    //
    // However, displacing white grand souls from black soul gems only
    // adds value when there exists a soul gem it can be displaced to,
    // thus it's preferable that we exit the soul processing anyway.
    for (const auto& [obj, entryData] : inventoryMap_) {
        const auto soulGem = obj->As<RE::TESSoulGem>();

        // Can happen if the type-cast failed, but all objects in the map
        // *should* be soul gems already.
        assert(soulGem != nullptr); 

        if (soulGem->GetMaximumCapacity() == soulGem->GetContainedSoul()) {
            ++maxFilledSoulGemsCount;
        }
    }

    if (inventoryMap_.size() <= 0) {
        casterInventoryStatus_ = InventoryStatus::NoSoulGemsOwned;
    } else if (inventoryMap_.size() == maxFilledSoulGemsCount) {
        casterInventoryStatus_ = InventoryStatus::AllSoulGemsFilled;
    } else {
        casterInventoryStatus_ = InventoryStatus::HasSoulGemsToFill;
    }

    isInventoryMapDirty_ = false;
}
