#include "SoulTrapFix.hpp"

#include <queue>

#include <xbyak/xbyak.h>

#include <SKSE/SKSE.h>
#include <REL/Relocation.h>

#include <RE/A/Actor.h>
#include <RE/B/BGSDefaultObjectManager.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESSoulGem.h>
#include <RE/B/BGSKeyword.h>
#include <RE/S/SoulLevels.h>
#include <RE/M/Misc.h>

#include "config/SoulGemConfig.hpp"
#include "Victim.hpp"

namespace Native {
    void* getManager() {
        using func_t = decltype(getManager);
        REL::Relocation<func_t> func{REL::ID{37916}}; // SkyrimSE.exe + 0x636c40 (v1.5.97.0)
        return func();
    }

    int incrementStat(void* manager, RE::Actor* const* const victim) {
        using func_t = decltype(incrementStat);
        REL::Relocation<func_t> func{REL::ID{37912}}; // SkyrimSE.exe + 0x6363e0 (v1.5.97.0)
        return func(manager, victim);
    }

    void incrementSoulsTrappedStat(RE::Actor* const victimActor) {
        void* manager = getManager();
        incrementStat(manager, &victimActor);
    }
}

int _indexOfFirstOwnedObjectInList(RE::TESObjectREFR::InventoryCountMap& inventoryCountMap, const std::vector<RE::TESSoulGem*>& objectsToSearch) {
    for (int i = 0; i < objectsToSearch.size(); ++i) {
        const auto boundObject = objectsToSearch[i]->As<RE::TESBoundObject>();

        if (inventoryCountMap[boundObject] > 0) {
            return i;
        }
    }

    return -1;
}

void _debugNotification(const char* message, RE::Actor* const caster, const Victim& victim) {
    if (caster->IsPlayerRef() && victim.actor() != nullptr) {
        RE::DebugNotification(message);
    }
}

bool trapSoul(RE::Actor* const casterActor, RE::Actor* const victimActor) {
    using namespace std::literals;
    namespace logger = SKSE::log;

    if (casterActor == nullptr || victimActor == nullptr || casterActor->IsDead(false) || !victimActor->IsDead(false)) {
        return false;
    }

    std::priority_queue<Victim, std::vector<Victim>, std::greater<Victim>> victims; // We need this to handle displaced souls.

    victims.emplace(victimActor);

    const SoulGemConfig& config = SoulGemConfig::getInstance();

    bool isSoulTrapSuccessful = false;

    while (!victims.empty()) {
        const Victim victim = victims.top();
        victims.pop();

        auto soulGemCountMap = casterActor->GetInventoryCounts([](RE::TESBoundObject& boundObject) {
            return boundObject.IsSoulGem();;
        });

        if (victim.soulSize() == SoulSize::Black) {
            const auto& emptySoulGems = config.getSoulGemsWith(SoulSize::Black, SoulSize::None);
            const auto& filledSoulGems = config.getSoulGemsWith(SoulSize::Black, SoulSize::Black);

            const int firstOwnedIndex = _indexOfFirstOwnedObjectInList(soulGemCountMap, emptySoulGems);

            if (firstOwnedIndex >= 0) {
                const auto soulGemToRemove = emptySoulGems[firstOwnedIndex];
                const auto soulGemToAdd = filledSoulGems[firstOwnedIndex];

                casterActor->AddObjectToContainer(soulGemToAdd, nullptr, 1, nullptr);
                casterActor->RemoveItem(soulGemToRemove, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);

                isSoulTrapSuccessful = true;
                goto PROCESS_NEXT_ENTRY;
            }
        } else {
            for (int soulCapacity = victim.soulSize(); soulCapacity <= SoulSize::Grand; ++soulCapacity) {
                const auto& targetSoulGems = config.getSoulGemsWith(toSoulSize(soulCapacity), victim.soulSize());

                for (int containedSoulSize = SoulSize::None; containedSoulSize < victim.soulSize(); ++containedSoulSize) {
                    const auto& sourceSoulGems = config.getSoulGemsWith(toSoulSize(soulCapacity), toSoulSize(containedSoulSize));
                    const int firstOwnedIndex = _indexOfFirstOwnedObjectInList(soulGemCountMap, sourceSoulGems);

                    if (firstOwnedIndex >= 0) {
                        const auto soulGemToRemove = sourceSoulGems[firstOwnedIndex];
                        const auto soulGemToAdd = targetSoulGems[firstOwnedIndex];

                        casterActor->AddObjectToContainer(soulGemToAdd, nullptr, 1, nullptr);
                        casterActor->RemoveItem(soulGemToRemove, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);

                        if (containedSoulSize > SoulSize::None) {
                            victims.emplace(static_cast<SoulSize>(containedSoulSize));
                        }

                        isSoulTrapSuccessful = true;
                        goto PROCESS_NEXT_ENTRY;
                    }
                }
            }

            // Shrinking
            for (int soulCapacity = victim.soulSize() - 1; soulCapacity >= SoulSize::Petty; --soulCapacity) {
                const auto& targetSoulGems = config.getSoulGemsWith(toSoulSize(soulCapacity), toSoulSize(soulCapacity));

                for (int containedSoulSize = SoulSize::None; containedSoulSize < victim.soulSize(); ++containedSoulSize) {
                    const auto& sourceSoulGems = config.getSoulGemsWith(toSoulSize(soulCapacity), toSoulSize(containedSoulSize));
                    const int firstOwnedIndex = _indexOfFirstOwnedObjectInList(soulGemCountMap, sourceSoulGems);

                    if (firstOwnedIndex >= 0) {
                        const auto soulGemToRemove = sourceSoulGems[firstOwnedIndex];
                        const auto soulGemToAdd = targetSoulGems[firstOwnedIndex];

                        casterActor->AddObjectToContainer(soulGemToAdd, nullptr, 1, nullptr);
                        casterActor->RemoveItem(soulGemToRemove, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);

                        if (containedSoulSize > SoulSize::None) {
                            victims.emplace(static_cast<SoulSize>(containedSoulSize));
                        }

                        isSoulTrapSuccessful = true;
                        goto PROCESS_NEXT_ENTRY;
                    }
                }
            }
        }
        // Use a goto label to break out of the deep loops.
    PROCESS_NEXT_ENTRY:
        if (isSoulTrapSuccessful) {
            _debugNotification("$YASTM_Notification_SoulCaptured", casterActor, victim);
            Native::incrementSoulsTrappedStat(victimActor);
        }
    }

    return isSoulTrapSuccessful;
}

void _handleMessage(SKSE::MessagingInterface::Message* message) {
    using namespace std::literals;
    namespace logger = SKSE::log;

    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        SoulGemConfig::getInstance().createSoulGemMap(RE::TESDataHandler::GetSingleton());
    }
}

bool yastm::installSoulTrapFix() {
    namespace logger = SKSE::log;

    SoulGemConfig::getInstance().loadConfig();

    auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener(_handleMessage);

    const REL::ID soulTrap1_id{37863};

    struct TrapSoulCall : Xbyak::CodeGenerator {
        explicit TrapSoulCall(const REL::ID& soulTrap1_id) {
            Xbyak::Label trapSoulLabel;
            Xbyak::Label returnLabel;

            mov(rdx, r9); // victim
            mov(rcx, r8); // caster

            call(ptr[rip + trapSoulLabel]);

            jmp(ptr[rip + returnLabel]);

            L(trapSoulLabel);
            dq(reinterpret_cast<std::uint64_t>(trapSoul));

            L(returnLabel);
            dq(soulTrap1_id.address() + 0x256);
        }
    };

    TrapSoulCall patch{soulTrap1_id};
    patch.ready();

    logger::info("[CHARGE] Patch size: {}", patch.getSize());

    auto& trampoline = SKSE::GetTrampoline();
    trampoline.write_branch<5>(
        soulTrap1_id.address() + 0x1f,
        trampoline.allocate(patch)
    );

    return true;
}
