#include "TrapSoulFix.hpp"

#include <deque>
#include <mutex>
#include <queue>
#include <cassert>

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
#include "utilities/printerror.hpp"
#include "utilities/TESSoulGem.hpp"
#include "utilities/Timer.hpp"

using namespace std::literals;

using VictimsQueue = std::priority_queue<Victim, std::deque<Victim>>;

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
     * @brief Returns the remaining "raw" soul size of the actor.
     *
     * The raw soul size is the actual capacity of the soul. They're mapped to
     * the enumerated soul sizes as follows:
     *
     * - None = 0
     * - Petty = 250
     * - Lesser = 500
     * - Common = 1000
     * - Greater = 2000
     * - Grand = 3000
     *
     * @returns 0 if the actor has already been soul trapped, otherwise returns
     * their raw soul size.
     */
    RawSoulSize getRemainingRawSoulSize(RE::Actor* const actor)
    {
        using func_t = decltype(getRemainingRawSoulSize);
        REL::Relocation<func_t> func{
            REL::ID{37861}}; // SkyrimSE.exe + 0x634830 (v1.5.97.0)
        return func(actor);
    }
}

/**
 * @brief Stores the data for various soul trap variables so we don't end up
 * with functions needing half a dozen arguments.
 */
class _SoulTrapData {
    static const std::size_t MAX_NOTIFICATION_COUNT = 1;
    std::size_t _notifyCount;
    bool _isStatIncremented;

    template <typename MessageKey>
    void _notify(const MessageKey message)
    {
        if (_notifyCount < MAX_NOTIFICATION_COUNT &&
            config.allowNotifications) {
            RE::DebugNotification(getMessage(message));
            ++_notifyCount;
        }
    }

    void _incrementSoulsTrappedStat(RE::Actor* const victim)
    {
        if (!_isStatIncremented) {
            void* manager = native::getStatManager();
            native::incrementStat(manager, &victim);
            _isStatIncremented = true;
        }
    }

public:
    RE::Actor* const caster;
    VictimsQueue victims;
    const YASTMConfig::Snapshot config;

    _SoulTrapData(RE::Actor* const caster)
        : caster{caster}
        , config{YASTMConfig::getInstance().createSnapshot()}
        , _notifyCount{0}
        , _isStatIncremented{false}
    {}

    _SoulTrapData(const _SoulTrapData&) = delete;
    _SoulTrapData(_SoulTrapData&&) = delete;
    _SoulTrapData& operator=(const _SoulTrapData&) = delete;
    _SoulTrapData& operator=(_SoulTrapData&&) = delete;

    void notifySoulTrapFailure(const SoulTrapFailureMessage message)
    {
        if (caster->IsPlayerRef()) {
            _notify(message);
        }
    }

    void notifySoulTrapSuccess(
        const SoulTrapSuccessMessage message,
        const Victim& victim)
    {
        if (caster->IsPlayerRef() && victim.isPrimarySoul()) {
            _notify(message);
            _incrementSoulsTrappedStat(victim.actor());
        }
    }
};

struct _SoulTrapLoopData {
    const Victim victim;
    const RE::TESObjectREFR::InventoryItemMap inventoryMap;

    explicit _SoulTrapLoopData(
        const Victim& victim,
        RE::TESObjectREFR::InventoryItemMap&& inventoryMap)
        : victim{victim}
        , inventoryMap{std::move(inventoryMap)}
    {}

    _SoulTrapLoopData(const _SoulTrapLoopData&) = delete;
    _SoulTrapLoopData(_SoulTrapLoopData&&) = delete;
    _SoulTrapLoopData& operator=(const _SoulTrapLoopData&) = delete;
    _SoulTrapLoopData& operator=(_SoulTrapLoopData&&) = delete;
};

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
    if (originalExtraList != nullptr) {
        // Inherit ownership.
        if (const auto owner = originalExtraList->GetOwner(); owner) {
            const auto newExtraList = new RE::ExtraDataList{};
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
    LOG_TRACE("Trapping black soul..."sv);

    const bool isSoulTrapped = _fillSoulGem(
        dl.victim.soulSize(),
        SoulSize::None,
        dl.victim.soulSize(),
        d,
        dl);

    if (isSoulTrapped) {
        d.notifySoulTrapSuccess(
            SoulTrapSuccessMessage::SoulCaptured,
            dl.victim);

        return true;
    }

    return false;
}

bool _trapFullSoul(_SoulTrapData& d, _SoulTrapLoopData& dl)
{
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
        for (int soulCapacity = static_cast<int>(dl.victim.soulSize());
             soulCapacity <= maxSoulCapacityToSearch;
             ++soulCapacity) {
            const auto& targetSoulGems = config.getSoulGemsWith(
                static_cast<SoulSize>(soulCapacity),
                dl.victim.soulSize());

            for (int containedSoulSize = static_cast<int>(SoulSize::None);
                 containedSoulSize < maxContainedSoulSizeToSearch;
                 ++containedSoulSize) {
                LOG_TRACE_FMT(
                    "Looking up soul gems with capacity = {}, containedSoulSize = {}"sv,
                    soulCapacity,
                    containedSoulSize);

                const auto& sourceSoulGems = config.getSoulGemsWith(
                    static_cast<SoulSize>(soulCapacity),
                    static_cast<SoulSize>(containedSoulSize));

                const bool result =
                    _fillSoulGem(sourceSoulGems, targetSoulGems, d, dl);

                if (result) {
                    if (d.config.allowRelocation &&
                        containedSoulSize > SoulSize::None) {
                        d.notifySoulTrapSuccess(
                            SoulTrapSuccessMessage::SoulDisplaced,
                            dl.victim);
                        d.victims.emplace(
                            static_cast<SoulSize>(containedSoulSize));
                    } else {
                        d.notifySoulTrapSuccess(
                            SoulTrapSuccessMessage::SoulCaptured,
                            dl.victim);
                    }

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
        for (int containedSoulSize = static_cast<int>(SoulSize::None);
             containedSoulSize < maxContainedSoulSizeToSearch;
             ++containedSoulSize) {
            for (int soulCapacity = static_cast<int>(dl.victim.soulSize());
                 soulCapacity <= maxSoulCapacityToSearch;
                 ++soulCapacity) {
                LOG_TRACE_FMT(
                    "Looking up soul gems with capacity = {}, containedSoulSize = {}"sv,
                    soulCapacity,
                    containedSoulSize);

                const bool result = _fillSoulGem(
                    static_cast<SoulSize>(soulCapacity),
                    static_cast<SoulSize>(containedSoulSize),
                    dl.victim.soulSize(),
                    d,
                    dl);

                if (result) {
                    if (d.config.allowRelocation &&
                        containedSoulSize > SoulSize::None) {
                        d.notifySoulTrapSuccess(
                            SoulTrapSuccessMessage::SoulDisplaced,
                            dl.victim);
                        d.victims.emplace(
                            static_cast<SoulSize>(containedSoulSize));
                    } else {
                        d.notifySoulTrapSuccess(
                            SoulTrapSuccessMessage::SoulCaptured,
                            dl.victim);
                    }

                    return true;
                }
            }
        }
    }

    return false;
}

bool _trapShrunkSoul(_SoulTrapData& d, _SoulTrapLoopData& dl)
{
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

    // Use a signed int instead of size_t to prevent a possible underflow issue.
    for (int soulCapacity = dl.victim.soulSize() - 1;
         soulCapacity > SoulSize::None;
         --soulCapacity) {
        const auto& targetSoulGems = config.getSoulGemsWith(
            static_cast<SoulSize>(soulCapacity),
            static_cast<SoulSize>(soulCapacity));

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
            d.config.allowDisplacement ? static_cast<SoulSize>(soulCapacity)
                                       : SoulSize::Petty;

        for (int containedSoulSize = static_cast<int>(SoulSize::None);
             containedSoulSize < maxContainedSoulSizeToSearch;
             ++containedSoulSize) {
            LOG_TRACE_FMT(
                "Looking up soul gems with capacity = {}, containedSoulSize = {}"sv,
                soulCapacity,
                containedSoulSize);

            const auto& sourceSoulGems = config.getSoulGemsWith(
                static_cast<SoulSize>(soulCapacity),
                static_cast<SoulSize>(containedSoulSize));

            const bool isFillSuccessful =
                _fillSoulGem(sourceSoulGems, targetSoulGems, d, dl);

            if (isFillSuccessful) {
                d.notifySoulTrapSuccess(
                    SoulTrapSuccessMessage::SoulShrunk,
                    dl.victim);

                if (d.config.allowRelocation &&
                    containedSoulSize > SoulSize::None) {
                    d.victims.emplace(static_cast<SoulSize>(containedSoulSize));
                }

                return true;
            }
        }
    }

    return false;
}

bool _trapSplitSoul(_SoulTrapData& d, _SoulTrapLoopData& dl)
{
    LOG_TRACE("Trapping split white soul..."sv);

    const YASTMConfig& config = YASTMConfig::getInstance();

    const SoulSize maxContainedSoulSizeToSearch =
        d.config.allowDisplacement ? static_cast<SoulSize>(dl.victim.soulSize())
                                   : SoulSize::Petty;

    const auto& targetSoulGems = config.getSoulGemsWith(
        static_cast<SoulSize>(dl.victim.soulSize()),
        static_cast<SoulSize>(dl.victim.soulSize()));

    for (int containedSoulSize = static_cast<int>(SoulSize::None);
         containedSoulSize < maxContainedSoulSizeToSearch;
         ++containedSoulSize) {
        LOG_TRACE_FMT(
            "Looking up soul gems with capacity = {}, containedSoulSize = {}"sv,
            dl.victim.soulSize(),
            containedSoulSize);

        const auto& sourceSoulGems = config.getSoulGemsWith(
            static_cast<SoulSize>(dl.victim.soulSize()),
            static_cast<SoulSize>(containedSoulSize));

        const bool isFillSuccessful =
            _fillSoulGem(sourceSoulGems, targetSoulGems, d, dl);

        if (isFillSuccessful) {
            d.notifySoulTrapSuccess(
                SoulTrapSuccessMessage::SoulSplit,
                dl.victim);

            if (d.config.allowRelocation &&
                containedSoulSize > SoulSize::None) {
                d.victims.emplace(static_cast<SoulSize>(containedSoulSize));
            }

            return true;
        }
    }

    return false;
}

void _splitSoul(const Victim& victim, VictimsQueue& victimQueue)
{
    // Raw Soul Sizes:
    // - Grand   = 3000 = Greater + Common
    // - Greater = 2000 = Common + Common
    // - Common  = 1000 = Lesser + Lesser
    // - Lesser  = 500  = Petty + Petty
    // - Petty   = 250
    switch (victim.soulSize()) {
    case SoulSize::Black:
        [[fallthrough]];
    case SoulSize::Grand:
        victimQueue.emplace(victim.actor(), SoulSize::Greater);
        victimQueue.emplace(victim.actor(), SoulSize::Common);
        break;
    case SoulSize::Greater:
        victimQueue.emplace(victim.actor(), SoulSize::Common);
        victimQueue.emplace(victim.actor(), SoulSize::Common);
        break;
    case SoulSize::Common:
        victimQueue.emplace(victim.actor(), SoulSize::Lesser);
        victimQueue.emplace(victim.actor(), SoulSize::Lesser);
        break;
    case SoulSize::Lesser:
        victimQueue.emplace(victim.actor(), SoulSize::Petty);
        victimQueue.emplace(victim.actor(), SoulSize::Petty);
        break;
    }
}

/**
 * @brief This logs the "enter" and "exit" messages upon construction and
 * destruction, respectively.
 *
 * It is also a timer object which will print the lifetime of this wrapper
 * object if profiling is enabled. 
 */
class _TrapSoulWrapper : public Timer {
public:
    explicit _TrapSoulWrapper()
    {
        LOG_TRACE("Entering YASTM trap soul function"sv);
    }

    virtual ~_TrapSoulWrapper()
    {
        const auto elapsedTime = elapsed();

        if (YASTMConfig::getInstance().getGlobalBool(
                ConfigKey::AllowProfiling)) {
            LOG_INFO_FMT("Time to trap soul: {:.7f} seconds", elapsedTime);
            RE::DebugNotification(fmt::format(getMessage(MiscMessage::TimeTakenToTrapSoul), elapsedTime).c_str());
        }

        LOG_TRACE("Exiting YASTM trap soul function"sv);
    }
};

std::mutex _trapSoulMutex; /* Process only one soul trap at a time. */

bool trapSoul(RE::Actor* const caster, RE::Actor* const victim)
{
    // This logs the "enter" and "exit" messages upon construction and
    // destruction, respectively.
    //
    // Also prints the time taken to run the function if profiling is enabled
    // (timer will still run if profiling is disabled, just with no visible
    // output).
    _TrapSoulWrapper wrapper;

    if (caster == nullptr) {
        LOG_TRACE("Caster is null."sv);
        return false;
    }

    if (victim == nullptr) {
        LOG_TRACE("Victim is null."sv);
        return false;
    }

    if (caster->IsDead(false)) {
        LOG_TRACE("Caster is dead."sv);
        return false;
    }

    if (!victim->IsDead(false)) {
        LOG_TRACE("Victim is not dead."sv);
        return false;
    }

    // We begin the mutex here since we're checking isSoulTrapped status next.
    std::lock_guard<std::mutex> guard{_trapSoulMutex};

    if (native::getRemainingRawSoulSize(victim) == RawSoulSize::None) {
        LOG_TRACE("Victim has already been soul trapped."sv);
        return false;
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
                d.caster->GetInventory([](const RE::TESBoundObject& object) {
                    return object.IsSoulGem();
                })};

            d.victims.pop();

            LOG_TRACE_FMT("Processing soul trap victim: {}", dl.victim);

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
            } else if (dl.victim.isSplitSoul()) {
                assert(d.config.allowSplitting);
                assert(!d.config.allowShrinking);

                if (_trapSplitSoul(d, dl)) {
                    isSoulTrapSuccessful = true;
                    continue; // Process next soul.
                }

                _splitSoul(dl.victim, d.victims);
                continue; // Process next soul.
            } else {
                if (_trapFullSoul(d, dl)) {
                    isSoulTrapSuccessful = true;
                    continue; // Process next soul.
                }

                // If we've reached this point, we start reducing the size of
                // the soul.
                //
                // Standard soul shrinking is prioritized over soul splitting.
                // Enabling both will implicitly turn off soul splitting.
                if (d.config.allowShrinking) {
                    if (_trapShrunkSoul(d, dl)) {
                        isSoulTrapSuccessful = true;

                        continue; // Process next soul.
                    }
                } else if (d.config.allowSplitting) {
                    _splitSoul(dl.victim, d.victims);

                    continue; // Process next soul.
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
                    d.notifySoulTrapFailure(
                        SoulTrapFailureMessage::NoSuitableSoulGem);
                } else {
                    d.notifySoulTrapFailure(
                        SoulTrapFailureMessage::NoSoulGemLargeEnough);
                }
            } else {
                d.notifySoulTrapFailure(
                    SoulTrapFailureMessage::NoSoulGemsAvailable);
            }
        }

        return isSoulTrapSuccessful;
    } catch (const std::exception& error) {
        LOG_ERROR(error.what());
    }

    return false;
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

bool installTrapSoulFix(const SKSE::LoadInterface* loadInterface)
{
    try {
        auto& config = YASTMConfig::getInstance();
        config.loadDllDependencies(loadInterface);
        config.loadConfig();
    } catch (const std::exception& error) {
        printError(error);
        LOG_ERROR("Not installing trapSoul patch.");

        return false;
    }

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
