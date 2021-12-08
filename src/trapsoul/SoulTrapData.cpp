#include "SoulTrapData.hpp"

void SoulTrapData::_resetInventoryData()
{
    std::size_t maxFilledSoulGemsCount = 0;

    // This should be a move.
    _inventoryMap =
        getInventoryFor(_caster, [&](const RE::TESBoundObject& obj) {
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
    for (const auto& [obj, entryData] : _inventoryMap) {
        const auto soulGem = obj->As<RE::TESSoulGem>();

        if (soulGem->GetMaximumCapacity() == soulGem->GetContainedSoul()) {
            ++maxFilledSoulGemsCount;
        }
    }

    if (_inventoryMap.size() <= 0) {
        _casterInventoryStatus = InventoryStatus::NoSoulGemsOwned;
    } else if (_inventoryMap.size() == maxFilledSoulGemsCount) {
        _casterInventoryStatus = InventoryStatus::AllSoulGemsFilled;
    } else {
        _casterInventoryStatus = InventoryStatus::HasSoulGemsToFill;
    }

    _isInventoryMapDirty = false;
}
