#include "TrapSoulFix.hpp"

#include <mutex>
#include <queue>

#include <xbyak/xbyak.h>

#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

#include <RE/A/Actor.h>
#include <RE/B/BGSDefaultObjectManager.h>
#include <RE/B/BGSKeyword.h>
#include <RE/M/Misc.h>
#include <RE/S/SoulLevels.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESSoulGem.h>

#include "global.hpp"
#include "messages.hpp"
#include "Victim.hpp"
#include "config/YASTMConfig.hpp"
#include "formatters/TESSoulGem.hpp"
#include "utilities/TESSoulGem.hpp"

using VictimsQueue =
    std::priority_queue<Victim, std::vector<Victim>, std::greater<Victim>>;

namespace native {
    /**
     * @brief Returns a pointer to the "stat manager".
     * (I have no idea exactly what this is, besides knowing what happens when
     * it's removed).
     */
    void* getStatManager()
    {
        using func_t = decltype(getStatManager);
        REL::Relocation<func_t> func{
            REL::ID{37916}}; // SkyrimSE.exe + 0x636c40 (v1.5.97.0)
        return func();
    }

    /**
     * @brief Increments the Souls Trapped stat given the manager and the
     * victim.
     * (I have no idea exactly what this is, besides knowing what happens when
     * it's removed).
     */
    int incrementStat(void* manager, RE::Actor* const* const victim)
    {
        using func_t = decltype(incrementStat);
        REL::Relocation<func_t> func{
            REL::ID{37912}}; // SkyrimSE.exe + 0x6363e0 (v1.5.97.0)
        return func(manager, victim);
    }

    /**
     * @brief Checks the soul trap status of the given actor.
     *
     * Note that this function can return weird numbers I can't figure out the
     * significance of. It's not important since all we need this for is check
     * if it's 0 or not, but it would be nice to know regardless.
     *  
     * @return 0 if it has been previously soul trapped. Weird numbers if not.
     */
    std::uint64_t soulTrapVictimStatus(RE::Actor* const actor)
    {
        using func_t = decltype(soulTrapVictimStatus);
        REL::Relocation<func_t> func{
            REL::ID{37861}}; // SkyrimSE.exe + 0x634830 (v1.5.97.0)
        return func(actor);
    }
}

void _incrementSoulsTrappedStat(
    RE::Actor* const caster,
    RE::Actor* const victim)
{
    if (caster == nullptr || victim == nullptr || !caster->IsPlayerRef()) {
        return;
    }

    void* manager = native::getStatManager();
    native::incrementStat(manager, &victim);
}

/**
 * @brief Stores the data for various soul trap variables so we don't end up
 * with functions needing half a dozen arguments.
 */
struct _SoulTrapData {
public:
    RE::Actor* const caster;
    VictimsQueue victims;
    const YASTMConfig::Snapshot config;

    _SoulTrapData(RE::Actor* const caster)
        : caster{caster}
        , config{YASTMConfig::getInstance().createSnapshot()}
    {}
};

struct _SoulTrapLoopData {
    const Victim victim;
    const RE::TESObjectREFR::InventoryItemMap inventoryMap;
};

void _debugNotification(const char* message, _SoulTrapData& d)
{
    if (!d.config.allowNotifications || d.caster == nullptr ||
        !d.caster->IsPlayerRef()) {
        return;
    }

    RE::DebugNotification(message);
}

void _debugNotification(const Message message, _SoulTrapData& d)
{
    _debugNotification(getMessage(message), d);
}

void _debugNotification(
    const char* message,
    _SoulTrapData& d,
    _SoulTrapLoopData& dl)
{
    if (dl.victim.isDisplacedSoul()) {
        return;
    }

    _debugNotification(message, d);
}

void _debugNotification(
    const Message message,
    _SoulTrapData& d,
    _SoulTrapLoopData& dl)
{
    _debugNotification(getMessage(message), d, dl);
}

struct _SearchResult {
    const std::size_t index;
    const RE::TESObjectREFR::Count itemCount;
    RE::InventoryEntryData* entryData;
};

std::optional<_SearchResult> _findFirstOwnedObjectInList(
    const RE::TESObjectREFR::InventoryItemMap& inventoryMap,
    const std::vector<RE::TESSoulGem*>& objectsToSearch)
{
    for (std::size_t i = 0; i < objectsToSearch.size(); ++i) {
        const auto boundObject = objectsToSearch[i]->As<RE::TESBoundObject>();

        if (inventoryMap.contains(boundObject)) {
            if (const auto& data = inventoryMap.at(boundObject);
                data.first > 0) {
                return std::make_optional<_SearchResult>(
                    i,
                    data.first,
                    data.second.get());
            }
        }
    }

    return std::nullopt;
}

[[nodiscard]] RE::ExtraDataList*
    _getFirstExtraDataList(RE::InventoryEntryData* const entryData)
{
    const auto extraLists = entryData->extraLists;

    if (extraLists == nullptr || extraLists->empty()) {
        return nullptr;
    }

    return extraLists->front();
}

/**
 * @brief Creates a new ExtraDataList, copying some properties from the
 * original.
 */
[[nodiscard]] RE::ExtraDataList*
    _createExtraDataListFromOriginal(RE::ExtraDataList* const originalExtraList)
{
    if (originalExtraList) {
        // Inherit ownership.
        if (const auto owner = originalExtraList->GetOwner(); owner) {
            auto newExtraList = new RE::ExtraDataList{};
            newExtraList->SetOwner(owner);
            return newExtraList;
        }
    }

    return nullptr;
}

void _replaceSoulGem(
    RE::TESSoulGem* const soulGemToAdd,
    RE::TESSoulGem* const soulGemToRemove,
    RE::InventoryEntryData* const soulGemToRemoveEntryData,
    _SoulTrapData& d)
{
    using namespace std::literals;

    RE::ExtraDataList* oldExtraList = nullptr;
    RE::ExtraDataList* newExtraList = nullptr;

    if (d.config.allowExtraSoulRelocation || d.config.preserveOwnership) {
        oldExtraList = _getFirstExtraDataList(soulGemToRemoveEntryData);
    }

    if (d.config.allowExtraSoulRelocation && oldExtraList != nullptr) {
        const RE::SOUL_LEVEL soulLevel = oldExtraList->GetSoulLevel();

        if (soulLevel != RE::SOUL_LEVEL::kNone) {
            SoulSize soulSize;

            // Assume that soul gems that can hold black souls and contain a
            // grand soul are holding a black soul (original information is long
            // gone anyway).
            if (soulLevel == RE::SOUL_LEVEL::kGrand &&
                canHoldBlackSoul(soulGemToRemove)) {
                soulSize = SoulSize::Black;
            } else {
                soulSize = static_cast<SoulSize>(soulLevel);
            }

            // Add the extra soul into the queue.
            LOG_TRACE_FMT("Relocating extra soul of size: {}"sv, soulSize);
            d.victims.emplace(soulSize);
        }
    }

    if (d.config.preserveOwnership) {
        newExtraList = _createExtraDataListFromOriginal(oldExtraList);
    }

    LOG_TRACE_FMT(
        "Replacing soul gems in {}'s inventory"sv,
        d.caster->GetName());
    LOG_TRACE_FMT("- from: {}"sv, soulGemToRemove);
    LOG_TRACE_FMT("- to: {}"sv, soulGemToAdd);

    d.caster->AddObjectToContainer(soulGemToAdd, newExtraList, 1, nullptr);
    d.caster->RemoveItem(
        soulGemToRemove,
        1,
        RE::ITEM_REMOVE_REASON::kRemove,
        oldExtraList,
        nullptr);
}

bool _fillSoulGem(
    const std::vector<RE::TESSoulGem*>& sourceSoulGems,
    const std::vector<RE::TESSoulGem*>& targetSoulGems,
    _SoulTrapData& d,
    _SoulTrapLoopData& dl)
{
    const auto maybeFirstOwned =
        _findFirstOwnedObjectInList(dl.inventoryMap, sourceSoulGems);

    if (maybeFirstOwned.has_value()) {
        const auto& firstOwned = maybeFirstOwned.value();

        const auto soulGemToAdd = targetSoulGems[firstOwned.index];
        const auto soulGemToRemove = sourceSoulGems[firstOwned.index];

        _replaceSoulGem(soulGemToAdd, soulGemToRemove, firstOwned.entryData, d);

        return true;
    }

    return false;
}

bool _fillSoulGem(
    const SoulSize capacity,
    const SoulSize sourceContainedSoulSize,
    const SoulSize targetContainedSoulSize,
    _SoulTrapData& d,
    _SoulTrapLoopData& dl)
{
    const YASTMConfig& config = YASTMConfig::getInstance();

    const auto& sourceSoulGems =
        config.getSoulGemsWith(capacity, sourceContainedSoulSize);
    const auto& targetSoulGems =
        config.getSoulGemsWith(capacity, targetContainedSoulSize);

    return _fillSoulGem(sourceSoulGems, targetSoulGems, d, dl);
}

bool _trapBlackSoul(_SoulTrapData& d, _SoulTrapLoopData& dl)
{
    // Black souls are simple since they're all or none. Either you have a
    // black soul gem or you don't. Nothing fancy to account for.
    using namespace std::literals;

    LOG_TRACE("Trapping black soul..."sv);

    const bool isSoulTrapped = _fillSoulGem(
        dl.victim.soulSize(),
        SoulSize::None,
        dl.victim.soulSize(),
        d,
        dl);

    if (isSoulTrapped) {
        _debugNotification(Message::SoulCaptured, d, dl);
        _incrementSoulsTrappedStat(d.caster, dl.victim.actor());

        return true;
    }

    return false;
}

bool _trapFullSoul(_SoulTrapData& d, _SoulTrapLoopData& dl)
{
    using namespace std::literals;

    LOG_TRACE("Trapping full white soul..."sv);

    const YASTMConfig& config = YASTMConfig::getInstance();

    // When partial trapping is allowed, we search all soul sizes up to
    // Grand. If it's not allowed, we only look at soul gems with the same
    // soul size.
    //
    // Note: Loop range is end-INclusive.
    const SoulSize maxSoulCapacityToSearch =
        d.config.allowPartial ? SoulSize::Grand : dl.victim.soulSize();

    // When displacement is allowed, we search soul gems with contained soul
    // sizes up to one size lower than the incoming soul. If it's not
    // allowed, we only look up empty soul gems.
    //
    // Note: Loop range is end-EXclusive, so we set this to SoulSize::Petty
    // as the next lowest soul size after SoulSize::None.
    const SoulSize maxContainedSoulSizeToSearch =
        d.config.allowDisplacement ? dl.victim.soulSize() : SoulSize::Petty;

    if (d.config.allowRelocation) {
        // With soul relocation, we try to fit the soul into the soul gem by
        // utilizing the "best-fit" principle:
        //
        // We define "fit" to be :
        //
        //     fit = soulCapacity - containedSoulSize
        //
        // The lower the value of the "fit", the better fit it is.
        //
        // The best-fit soul gem is a fully-filled soul gem.
        // The worst-fit soul gem is an empty soul gem.
        //
        // When "fit" is equal, the soul gem closest in size to the given soul
        // size takes priority.
        //
        // To maximize the fit, the algorithm is described roughly as follows:
        //
        // Given a soul of size X, soul gem capacity C, and existing soul
        // size E:
        //
        // From C = X up to C = 5
        //     From E = 0 up to E = C - 1
        //         If HasSoulGem(Capacity = C, ExistingSoulSize = E)
        //             FillSoulGem(SoulSize = X, Capacity = C, ExistingSoulSize = E)
        //             Return
        //         Else
        //             Continue searching
        for (std::size_t soulCapacity = dl.victim.soulSize();
             soulCapacity <= maxSoulCapacityToSearch;
             ++soulCapacity) {
            const auto& targetSoulGems = config.getSoulGemsWith(
                toSoulSize(soulCapacity),
                dl.victim.soulSize());

            for (std::size_t containedSoulSize = SoulSize::None;
                 containedSoulSize < maxContainedSoulSizeToSearch;
                 ++containedSoulSize) {
                LOG_TRACE_FMT(
                    "Looking up soul gems with capacity = {}, containedSoulSize = {}"sv,
                    soulCapacity,
                    containedSoulSize);

                const auto& sourceSoulGems = config.getSoulGemsWith(
                    toSoulSize(soulCapacity),
                    toSoulSize(containedSoulSize));

                const bool result =
                    _fillSoulGem(sourceSoulGems, targetSoulGems, d, dl);

                if (result) {
                    if (d.config.allowRelocation &&
                        containedSoulSize > SoulSize::None) {
                        _debugNotification(Message::SoulDisplaced, d, dl);
                        d.victims.emplace(toSoulSize(containedSoulSize));
                    } else {
                        _debugNotification(Message::SoulCaptured, d, dl);
                    }

                    _incrementSoulsTrappedStat(d.caster, dl.victim.actor());

                    return true;
                }
            }
        }
    } else {
        // Without soul relocation, we need to minimize soul loss by displacing
        // the smallest soul first.
        //
        // The algorithm is described roughly as follows:
        //
        // Given a soul of size X, soul gem capacity C, and existing soul
        // size E:
        //
        // From E = 0 up to E = X - 1
        //     From C = X up to C = 5
        //         If HasSoulGem(Capacity = C, ExistingSoulSize = E)
        //             FillSoulGem(SoulSize = X, Capacity = C, ExistingSoulSize = E)
        //             Return
        //         Else
        //             Continue searching
        for (std::size_t containedSoulSize = SoulSize::None;
             containedSoulSize < maxContainedSoulSizeToSearch;
             ++containedSoulSize) {
            for (std::size_t soulCapacity = dl.victim.soulSize();
                 soulCapacity <= maxSoulCapacityToSearch;
                 ++soulCapacity) {
                LOG_TRACE_FMT(
                    "Looking up soul gems with capacity = {}, containedSoulSize = {}"sv,
                    soulCapacity,
                    containedSoulSize);

                const bool result = _fillSoulGem(
                    toSoulSize(soulCapacity),
                    toSoulSize(containedSoulSize),
                    dl.victim.soulSize(),
                    d,
                    dl);

                if (result) {
                    if (d.config.allowRelocation &&
                        containedSoulSize > SoulSize::None) {
                        _debugNotification(Message::SoulDisplaced, d, dl);
                        d.victims.emplace(toSoulSize(containedSoulSize));
                    } else {
                        _debugNotification(Message::SoulCaptured, d, dl);
                    }

                    _incrementSoulsTrappedStat(d.caster, dl.victim.actor());

                    return true;
                }
            }
        }
    }

    return false;
}

bool _trapShrunkSoul(_SoulTrapData& d, _SoulTrapLoopData& dl)
{
    using namespace std::literals;

    LOG_TRACE("Trapping shrunk white soul..."sv);

    const YASTMConfig& config = YASTMConfig::getInstance();

    // Avoid shrinking a soul more than necessary. Any soul we displace must be
    // smaller than the soul gem capacity itself, and shrunk souls always fully
    // fill the soul gem. This suggests that we generally lose more from
    // shrinking the soul than losing a displaced soul.
    //
    // Because of this, we don't have special prioritization for when soul
    // relocation is disabled.
    //
    // This algorithm matches the one for trapping full white souls when both
    // displacement and relocation are enabled, except that we iterate over soul
    // capacity in descending order.
    for (std::size_t soulCapacity = dl.victim.soulSize() - 1;
         soulCapacity > SoulSize::None;
         --soulCapacity) {
        const auto& targetSoulGems = config.getSoulGemsWith(
            toSoulSize(soulCapacity),
            toSoulSize(soulCapacity));

        // When displacement is allowed, we search soul gems with contained soul
        // sizes up to one size lower than the incoming soul. Since the incoming
        // soul size varies depending on the shrunk soul size, we put this
        // inside the loop.
        //
        // If it's not allowed, we only look up empty soul gems.
        //
        // Note: Loop range is end-EXclusive, so we set this to SoulSize::Petty
        // as the next lowest soul size after SoulSize::None.
        const SoulSize maxContainedSoulSizeToSearch =
            d.config.allowDisplacement ? toSoulSize(soulCapacity)
                                       : SoulSize::Petty;

        for (std::size_t containedSoulSize = SoulSize::None;
             containedSoulSize < maxContainedSoulSizeToSearch;
             ++containedSoulSize) {
            LOG_TRACE_FMT(
                "Looking up soul gems with capacity = {}, containedSoulSize = {}"sv,
                soulCapacity,
                containedSoulSize);

            const auto& sourceSoulGems = config.getSoulGemsWith(
                toSoulSize(soulCapacity),
                toSoulSize(containedSoulSize));

            const bool isFillSuccessful =
                _fillSoulGem(sourceSoulGems, targetSoulGems, d, dl);

            if (isFillSuccessful) {
                _debugNotification(Message::SoulShrunk, d, dl);
                _incrementSoulsTrappedStat(d.caster, dl.victim.actor());

                if (containedSoulSize > SoulSize::None) {
                    d.victims.emplace(toSoulSize(containedSoulSize));
                }

                return true;
            }
        }
    }

    return false;
}

std::mutex _trapSoulMutex; /* Process only one soul trap at a time. */

bool trapSoul(RE::Actor* const caster, RE::Actor* const victim)
{
    using namespace std::literals;

    /**
     * @brief Use this instead of a raw return value to wrap the return value so
     * that the exiting trace log will be printed as needed.
     */
    const auto wrapUpAndReturn = [](const bool returnValue) {
        LOG_TRACE("Exiting YASTM trap soul function"sv);
        return returnValue;
    };

    LOG_TRACE("Entering YASTM trap soul function"sv);

    if (caster == nullptr) {
        LOG_TRACE("Caster is null."sv);
        return wrapUpAndReturn(false);
    }

    if (victim == nullptr) {
        LOG_TRACE("Victim is null."sv);
        return wrapUpAndReturn(false);
    }

    if (caster->IsDead(false)) {
        LOG_TRACE("Caster is dead."sv);
        return wrapUpAndReturn(false);
    }

    if (!victim->IsDead(false)) {
        LOG_TRACE("Victim is not dead."sv);
        return wrapUpAndReturn(false);
    }

    // We begin the mutex here since we're checking isSoulTrapped status next.
    std::lock_guard<std::mutex> guard{_trapSoulMutex};

    if (native::soulTrapVictimStatus(victim) == 0) {
        LOG_TRACE("Victim has already been soul trapped."sv);
        return wrapUpAndReturn(false);
    }

    bool isSoulTrapSuccessful = false;

    try {
        // Initialize the data we're going to pass around to various functions.
        //
        // Includes:
        // - victims: a priority queue where largest souls are prioritized
        //            first. Needed for handling displaced souls.
        // - config:  a snapshot of the configuration so it would be immune to
        //            external changes for this particular call.
        _SoulTrapData d{caster};

        d.victims.emplace(victim);

        LOG_TRACE("Found configuration:"sv);
        LOG_TRACE_FMT("- Allow partial: {}"sv, d.config.allowPartial);
        LOG_TRACE_FMT("- Allow displacement: {}"sv, d.config.allowDisplacement);
        LOG_TRACE_FMT("- Allow relocation: {}"sv, d.config.allowRelocation);
        LOG_TRACE_FMT("- Allow shrinking: {}"sv, d.config.allowShrinking);
        LOG_TRACE_FMT(
            "- Allow extra soul relocation: {}"sv,
            d.config.allowExtraSoulRelocation);
        LOG_TRACE_FMT("- Preserve ownership: {}"sv, d.config.preserveOwnership);
        LOG_TRACE_FMT(
            "- Allow notifications: {}"sv,
            d.config.allowNotifications);

        bool casterHasAvailableSoulGems = true;

        while (!d.victims.empty()) {
            _SoulTrapLoopData dl{
                d.victims.top(),
                d.caster->GetInventory(
                    [](RE::TESBoundObject& a) { return a.IsSoulGem(); })};
            d.victims.pop();

            // Set it here so we don't have to pass half a dozen arguments
            // everywhere.
            //
            // Do NOT access these outside this loop as their scope ends once
            // we reach the end of this block. The values are NOT changed to
            // null once it falls outside this scope.

            if (dl.inventoryMap.size() <= 0) {
                // Caster doesn't have any soul gems. Stop looking.
                LOG_TRACE("Caster has no soul gems. Stop looking."sv);
                casterHasAvailableSoulGems = false;
                break;
            }

            if (dl.victim.soulSize() == SoulSize::Black) {
                if (_trapBlackSoul(d, dl)) {
                    isSoulTrapSuccessful = true;
                    continue; // Process next soul.
                }
            } else /* White souls */ {
                if (_trapFullSoul(d, dl)) {
                    isSoulTrapSuccessful = true;
                    continue; // Process next soul.
                }

                // If we failed the previous step, start shrinking.
                if (d.config.allowShrinking) {
                    if (_trapShrunkSoul(d, dl)) {
                        isSoulTrapSuccessful = true;

                        continue; // Process next soul.
                    }
                }
            }
        }

        if (isSoulTrapSuccessful) {
            // Flag the victim so we don't soul trap the same one multiple
            // times.
            if (RE::AIProcess* const process = victim->currentProcess;
                process) {
                if (process->middleHigh) {
                    LOG_TRACE("Flagging soul trapped victim..."sv);
                    process->middleHigh->unk325 = true;
                }
            }
        } else {
            if (casterHasAvailableSoulGems) {
                if (d.config.allowShrinking) {
                    _debugNotification(Message::NoSuitableSoulGem, d);
                } else {
                    _debugNotification(Message::NoSoulGemLargeEnough, d);
                }
            } else {
                _debugNotification(Message::NoSoulGemsAvailable, d);
            }
        }

        return wrapUpAndReturn(isSoulTrapSuccessful);
    } catch (const std::exception& error) {
        LOG_ERROR(error.what());
    }

    return wrapUpAndReturn(false);
}

void _handleMessage(SKSE::MessagingInterface::Message* message)
{
    using namespace std::literals;

    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        YASTMConfig::getInstance().processGameForms(
            RE::TESDataHandler::GetSingleton());
    }
}

/**
 * Check if memory has the expected bytes for patching.
 */
bool _isTrapSoulPatchable(
    const std::uintptr_t baseAddress,
    const std::uintptr_t callOffset,
    const std::uintptr_t returnOffset)
{
    const std::uint8_t expectedEntry[] = {
        // clang-format off
        // .text:000000014063491F
        0x48, 0x8b, 0xf2,                  // mov  rsi, rdx
        0x4c, 0x8b, 0xf1,                  // mov  r14, rcx
        0x40, 0x32, 0xff,                  // xor  dil, dil
        0x48, 0x8b, 0x01,                  // mov  rax, [rcx]
        0x33, 0xd2,                        // xor  edx, edx
        0xff, 0x90, 0xc8, 0x04, 0x00, 0x00 // call qword ptr [rax+4C8h]
        // clang-format on
    };

    const std::uint8_t expectedExit[] = {
        // clang-format off
        // .text:0000000140634B56
        0x4c, 0x8d, 0x5c, 0x24, 0x70, // lea r11, [rsp+98h+var_28]
        0x49, 0x8b, 0x5b, 0x38,       // mov rbx, [r11+38h]
        0x49, 0x8b, 0x6b, 0x40,       // mov rbp, [r11+40h]
        0x49, 0x8b, 0xe3,             // mov rsp, r11
        // clang-format on
    };

    if (std::memcmp(
            reinterpret_cast<std::uint8_t*>(
                static_cast<std::uintptr_t>(baseAddress + callOffset)),
            expectedEntry,
            sizeof expectedEntry) != 0) {
        LOG_CRITICAL(
            "[TRAPSOUL] Expected bytes for soul trap handling at call offset not found."sv);
        return false;
    }

    if (std::memcmp(
            reinterpret_cast<std::uint8_t*>(
                static_cast<std::uintptr_t>(baseAddress + returnOffset)),
            expectedExit,
            sizeof expectedExit) != 0) {
        LOG_CRITICAL(
            "[TRAPSOUL] Expected bytes for soul trap handling at return offset not found."sv);
        return false;
    }

    return true;
}

bool installTrapSoulFix()
{
    using namespace std::literals;

    YASTMConfig::getInstance().loadConfig();

    auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener(_handleMessage);

    const REL::ID soulTrap1_id{37863};
    constexpr std::uintptr_t callOffset = 0x1f;
    constexpr std::uintptr_t returnOffset = 0x256;

    if (!_isTrapSoulPatchable(
            soulTrap1_id.address(),
            callOffset,
            returnOffset)) {
        return false;
    }

    // This simply sets up the registers so they will be passed to our
    // replacement function correctly, and jumps back to our original function's
    // ending address.
    struct TrapSoulCall : Xbyak::CodeGenerator {
        explicit TrapSoulCall(
            const REL::ID& soulTrap1_id,
            const std::uintptr_t returnOffset)
        {
            Xbyak::Label trapSoulLabel;
            Xbyak::Label returnLabel;

            // Set up the arguments and call our function.
            mov(rdx, r9); // victim
            mov(rcx, r8); // caster

            call(ptr[rip + trapSoulLabel]);

            // Jump to original function end.
            jmp(ptr[rip + returnLabel]);

            L(trapSoulLabel);
            dq(reinterpret_cast<std::uint64_t>(trapSoul));

            L(returnLabel);
            dq(soulTrap1_id.address() + returnOffset);
        }
    };

    TrapSoulCall patch{soulTrap1_id, returnOffset};
    patch.ready();

    LOG_INFO_FMT("[TRAPSOUL] Patch size: {}"sv, patch.getSize());

    auto& trampoline = SKSE::GetTrampoline();
    trampoline.write_branch<5>(
        soulTrap1_id.address() + callOffset,
        trampoline.allocate(patch));

    return true;
}
