#include "misc.hpp"

UnorderedInventoryItemMap getInventoryFor(
    RE::TESObjectREFR* objectRef,
    std::function<bool(RE::TESBoundObject&)> a_filter)
{
    UnorderedInventoryItemMap results;

    auto invChanges = objectRef->GetInventoryChanges();
    if (invChanges && invChanges->entryList) {
        for (auto& entry : *invChanges->entryList) {
            if (entry && entry->object && a_filter(*entry->object)) {
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
        const auto ignore = [&](RE::TESBoundObject* a_object) {
            const auto it = results.find(a_object);
            const auto entryData =
                it != results.end() ? it->second.second.get() : nullptr;
            return entryData ? entryData->IsLeveled() : false;
        };

        container->ForEachContainerObject([&](RE::ContainerObject& a_entry) {
            auto obj = a_entry.obj;
            if (obj && !ignore(obj) && a_filter(*obj)) {
                auto it = results.find(obj);
                if (it == results.end()) {
                    [[maybe_unused]] auto insIt = results.emplace(
                        obj,
                        std::make_pair(
                            a_entry.count,
                            std::make_unique<RE::InventoryEntryData>(obj, 0)));
                    assert(insIt.second);
                } else {
                    it->second.first += a_entry.count;
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
    static const auto reusableSoulGemKeyword =
        RE::BGSDefaultObjectManager::GetSingleton()->GetObject<RE::BGSKeyword>(
            RE::DEFAULT_OBJECT::kKeywordReusableSoulGem);

    return reusableSoulGemKeyword;
}
