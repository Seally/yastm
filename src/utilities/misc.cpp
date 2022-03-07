#include "misc.hpp"

UnorderedInventoryItemMap getInventoryFor(
    RE::TESObjectREFR* const objectRef,
    std::function<bool(RE::TESBoundObject&)> filter)
{
    UnorderedInventoryItemMap results;

    auto invChanges = objectRef->GetInventoryChanges();
    if (invChanges && invChanges->entryList) {
        for (auto& entry : *invChanges->entryList) {
            if (entry && entry->object && filter(*entry->object)) {
                [[maybe_unused]] auto it = results.emplace(
                    entry->object,
                    std::make_pair(
                        entry->countDelta,
                        std::make_unique<RE::InventoryEntryData>(*entry)));
                assert(it.second);
            }
        }
    }

    auto container = objectRef->GetContainer();
    if (container) {
        const auto ignore = [&](RE::TESBoundObject* const object) {
            const auto it = results.find(object);
            const auto entryData =
                it != results.end() ? it->second.second.get() : nullptr;
            return entryData ? entryData->IsLeveled() : false;
        };

        container->ForEachContainerObject([&](RE::ContainerObject& entry) {
            auto obj = entry.obj;
            if (obj && !ignore(obj) && filter(*obj)) {
                auto it = results.find(obj);
                if (it == results.end()) {
                    [[maybe_unused]] auto insIt = results.emplace(
                        obj,
                        std::make_pair(
                            entry.count,
                            std::make_unique<RE::InventoryEntryData>(obj, 0)));
                    assert(insIt.second);
                } else {
                    it->second.first += entry.count;
                }
            }
            return true;
        });
    }

    return results;
}

RE::BGSKeyword* getReusableSoulGemKeyword()
{
    // I don't know why putting this in a .cpp file stops Visual Studio/MSVC
    // from thinking GetObject is a macro from wingdi.h.
    const auto defaultObjectManager =
        RE::BGSDefaultObjectManager::GetSingleton();

    assert(defaultObjectManager != nullptr);

    return defaultObjectManager->GetObject<RE::BGSKeyword>(
        RE::DEFAULT_OBJECT::kKeywordReusableSoulGem);
}
