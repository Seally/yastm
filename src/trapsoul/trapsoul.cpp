#include "trapsoul.hpp"

#include <deque>
#include <mutex>
#include <optional>
#include <queue>

#include <cassert>

#include <fmt/format.h>

#include <RE/A/Actor.h>
#include <RE/S/SoulsTrapped.h>
#include <RE/T/TESBoundObject.h>
#include <RE/T/TESObjectREFR.h>
#include <RE/T/TESSoulGem.h>

#include "../global.hpp"
#include "../messages.hpp"
#include "../SoulValue.hpp"
#include "types.hpp"
#include "InventoryStatus.hpp"
#include "SearchResult.hpp"
#include "SoulTrapData.hpp"
#include "Victim.hpp"
#include "../config/YASTMConfig.hpp"
#include "../formatters/TESSoulGem.hpp"
#include "../utilities/misc.hpp"
#include "../utilities/native.hpp"
#include "../utilities/printerror.hpp"
#include "../utilities/rng.hpp"
#include "../utilities/Timer.hpp"

using namespace std::literals;

namespace {
    std::optional<SearchResult> findFirstOwnedObjectInList_(
        const SoulTrapData::InventoryItemMap& inventoryMap,
        const SoulGemMap::IteratorPair& objectsToSearch)
    {
        const auto& [begin, end] = objectsToSearch;

        for (auto it = begin; it != end; ++it) {
            const auto boundObject = it->As<RE::TESBoundObject>();

            if (inventoryMap.contains(boundObject)) {
                if (const auto& data = inventoryMap.at(boundObject);
                    data.first > 0) {
                    return std::make_optional<SearchResult>(
                        it,
                        data.first,
                        data.second.get());
                }
            }
        }

        return std::nullopt;
    }

    [[nodiscard]] RE::ExtraDataList*
        getFirstExtraDataList_(RE::InventoryEntryData* const entryData)
    {
        const auto extraLists = entryData->extraLists;

        if (extraLists == nullptr || extraLists->empty()) {
            return nullptr;
        }

        return extraLists->front();
    }

    void replaceSoulGem_(
        RE::TESSoulGem* const soulGemToAdd,
        RE::TESSoulGem* const soulGemToRemove,
        RE::InventoryEntryData* const soulGemToRemoveEntryData,
        SoulTrapData& d)
    {
        RE::ExtraDataList* oldExtraList = nullptr;
        std::unique_ptr<RE::ExtraDataList> newExtraList;

        if (d.config[BC::AllowExtraSoulRelocation] ||
            d.config[BC::PreserveOwnership]) {
            oldExtraList = getFirstExtraDataList_(soulGemToRemoveEntryData);
        }

        if (d.config[BC::AllowExtraSoulRelocation] && oldExtraList != nullptr) {
            const RE::SOUL_LEVEL soulLevel = oldExtraList->GetSoulLevel();

            if (soulLevel != RE::SOUL_LEVEL::kNone) {
                SoulSize soulSize;

                // Assume that soul gems that can hold black souls and contain a
                // grand soul are holding a black soul (original information is
                // long gone anyway).
                if (soulLevel == RE::SOUL_LEVEL::kGrand &&
                    canHoldBlackSoul(soulGemToRemove)) {
                    soulSize = SoulSize::Black;
                } else {
                    soulSize = toSoulSize(soulLevel);
                }

                // Add the extra soul into the queue.
                LOG_TRACE_FMT("Relocating extra soul of size: {:t}", soulSize);
                d.victims().emplace(soulSize);
            }
        }

        if (d.config[BC::PreserveOwnership]) {
            newExtraList = createExtraDataListFromOriginal(oldExtraList);
        }

        LOG_TRACE_FMT(
            "Replacing soul gems in {}'s inventory",
            d.caster()->GetName());
        LOG_TRACE_FMT("- from: {:f}", *soulGemToRemove);
        LOG_TRACE_FMT("- to: {:f}", *soulGemToAdd);

        d.caster()->AddObjectToContainer(
            soulGemToAdd,
            newExtraList.release(), // Transfer ownership to the engine.
            1,
            nullptr);
        d.caster()->RemoveItem(
            soulGemToRemove,
            1,
            RE::ITEM_REMOVE_REASON::kRemove,
            oldExtraList,
            nullptr);
        d.setInventoryHasChanged();
    }

    bool fillSoulGem_(
        const SoulGemMap::IteratorPair& sourceSoulGems,
        const SoulSize targetContainedSoulSize,
        SoulTrapData& d)
    {
        const auto maybeFirstOwned =
            findFirstOwnedObjectInList_(d.inventoryMap(), sourceSoulGems);

        if (maybeFirstOwned.has_value()) {
            const auto& firstOwned = maybeFirstOwned.value();

            const auto soulGemToAdd =
                firstOwned.soulGemAt(targetContainedSoulSize);
            const auto soulGemToRemove = firstOwned.soulGem();

            replaceSoulGem_(
                soulGemToAdd,
                soulGemToRemove,
                firstOwned.entryData(),
                d);

            return true;
        }

        return false;
    }

    bool fillWhiteSoulGem_(
        const SoulGemCapacity capacity,
        const SoulSize sourceContainedSoulSize,
        const SoulSize targetContainedSoulSize,
        SoulTrapData& d)
    {
        const auto& soulGemMap = YASTMConfig::getInstance().soulGemMap();

        const auto& sourceSoulGems =
            soulGemMap.getSoulGemsWith(capacity, sourceContainedSoulSize);

        return fillSoulGem_(sourceSoulGems, targetContainedSoulSize, d);
    }

    bool fillBlackSoulGem_(SoulTrapData& d)
    {
        const auto& soulGemMap = YASTMConfig::getInstance().soulGemMap();
        const auto& sourceSoulGems =
            soulGemMap.getSoulGemsWith(SoulGemCapacity::Black, SoulSize::None);

        return fillSoulGem_(sourceSoulGems, SoulSize::Black, d);
    }

    bool tryReplaceBlackSoulInDualSoulGemWithWhiteSoul_(SoulTrapData& d)
    {
        const auto& soulGemMap = YASTMConfig::getInstance().soulGemMap();

        // Find our black-filled dual soul gem.
        const auto& sourceSoulGems =
            soulGemMap.getSoulGemsWith(SoulGemCapacity::Dual, SoulSize::Black);

        const auto maybeFirstOwned =
            findFirstOwnedObjectInList_(d.inventoryMap(), sourceSoulGems);

        // If the black-filled dual soul exists in the inventory and we can fill
        // an empty pure black soul gem, fill the dual soul gem with our white
        // soul.
        if (maybeFirstOwned.has_value() && fillBlackSoulGem_(d)) {
            const auto& firstOwned = maybeFirstOwned.value();

            const auto soulGemToAdd =
                firstOwned.soulGemAt(d.victim().soulSize());
            const auto soulGemToRemove = firstOwned.soulGem();

            replaceSoulGem_(
                soulGemToAdd,
                soulGemToRemove,
                firstOwned.entryData(),
                d);

            return true;
        }

        return false;
    }

    bool trapBlackSoul_(SoulTrapData& d)
    {
        LOG_TRACE("Trapping black soul...");

        // We try to trap black souls into black soul gems first. If that
        // succeeds, we can stop here.
        LOG_TRACE("Looking up pure empty black soul gems");
        const bool isSoulTrapped = fillBlackSoulGem_(d);

        if (isSoulTrapped) {
            d.notifySoulTrapSuccess(
                SoulTrapSuccessMessage::SoulCaptured,
                d.victim());

            return true;
        }

        const auto& soulGemMap = YASTMConfig::getInstance().soulGemMap();

        // When displacement is allowed, we search dual soul gems with a
        // contained soul size up to SoulSize::Grand to allow displacing white
        // grand souls.
        //
        // When displacement is NOT allowed, we search only for empty dual
        // soul gems.
        //
        // Note: Loop range is end-EXclusive, so we use the next lowest soul
        // sizes after our target (Grand => Black, None => Petty).
        const SoulSize maxContainedSoulSizeToSearch =
            d.config[BC::AllowSoulDisplacement] ? SoulSize::Black
                                                : SoulSize::Petty;

        // Perform the actual search for the appropriate dual soul gem.
        for (SoulSizeValue containedSoulSize = SoulSize::None;
             containedSoulSize < maxContainedSoulSizeToSearch;
             ++containedSoulSize) {
            LOG_TRACE_FMT(
                "Looking up dual soul gems with containedSoulSize = {:t}",
                containedSoulSize);

            const auto& sourceSoulGems = soulGemMap.getSoulGemsWith(
                SoulGemCapacity::Dual,
                containedSoulSize);

            const bool result =
                fillSoulGem_(sourceSoulGems, d.victim().soulSize(), d);

            if (result) {
                if (d.config[BC::AllowSoulRelocation] &&
                    containedSoulSize > SoulSize::None) {
                    d.notifySoulTrapSuccess(
                        SoulTrapSuccessMessage::SoulDisplaced,
                        d.victim());
                    d.victims().emplace(
                        static_cast<SoulSize>(containedSoulSize));
                } else {
                    d.notifySoulTrapSuccess(
                        SoulTrapSuccessMessage::SoulCaptured,
                        d.victim());
                }

                return true;
            }
        }

        return false;
    }

    bool trapFullSoul_(SoulTrapData& d)
    {
        LOG_TRACE("Trapping full white soul...");

        const auto& soulGemMap = YASTMConfig::getInstance().soulGemMap();

        // When partial trapping is allowed, we search all soul sizes up to
        // Grand. If it's not allowed, we only look at soul gems with the same
        // soul size.
        //
        // Note: Loop range is end-INclusive.
        const SoulGemCapacity maxSoulCapacityToSearch =
            d.config[BC::AllowPartiallyFillingSoulGems]
                ? SoulGemCapacity::LastWhite
                : toSoulGemCapacity(d.victim().soulSize());

        // When displacement is allowed, we search soul gems with contained soul
        // sizes up to one size lower than the incoming soul. If it's not
        // allowed, we only look up empty soul gems.
        //
        // Note: Loop range is end-EXclusive, so we set this to SoulSize::Petty
        // as the next lowest soul size after SoulSize::None.
        const SoulSize maxContainedSoulSizeToSearch =
            d.config[BC::AllowSoulDisplacement] ? d.victim().soulSize()
                                                : SoulSize::Petty;

        if (d.config[BC::AllowSoulRelocation]) {
            // With soul relocation, we try to fit the soul into the soul gem by
            // utilizing the "best-fit" principle:
            //
            // We define "fit" to be :
            //
            //     fit = capacity - containedSoulSize
            //
            // The lower the value of the "fit", the better fit it is.
            //
            // The best-fit soul gem is a fully-filled soul gem.
            // The worst-fit soul gem is an empty soul gem.
            //
            // When "fit" is equal, the soul gem closest in size to the given
            // soul size takes priority.
            //
            // To maximize the fit, the algorithm is described roughly as
            // follows:
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
            for (SoulGemCapacityValue capacity =
                     toSoulGemCapacity(d.victim().soulSize());
                 capacity <= maxSoulCapacityToSearch;
                 ++capacity) {
                for (SoulSizeValue containedSoulSize = SoulSize::None;
                     containedSoulSize < maxContainedSoulSizeToSearch;
                     ++containedSoulSize) {
                    LOG_TRACE_FMT(
                        "Looking up white soul gems with capacity = {:t}, "
                        "containedSoulSize = {:t}",
                        capacity,
                        containedSoulSize);

                    const auto& sourceSoulGems =
                        soulGemMap.getSoulGemsWith(capacity, containedSoulSize);

                    const bool result =
                        fillSoulGem_(sourceSoulGems, d.victim().soulSize(), d);

                    if (result) {
                        // We've checked for soul relocation already. No need to
                        // do that again here.
                        if (containedSoulSize > SoulSize::None) {
                            d.notifySoulTrapSuccess(
                                SoulTrapSuccessMessage::SoulDisplaced,
                                d.victim());
                            d.victims().emplace(
                                static_cast<SoulSize>(containedSoulSize));
                        } else {
                            d.notifySoulTrapSuccess(
                                SoulTrapSuccessMessage::SoulCaptured,
                                d.victim());
                        }

                        return true;
                    }
                }
            }

            // Look up if there are any black souls stored in dual soul gems. If
            // any exists, check if there is an empty pure black soul gem and
            // fill it, then fill the dual soul gem with the new soul.
            //
            // This is handled without using the victims queue to avoid an
            // infinite loop from black souls displacing white souls and white
            // souls displacing black souls.
            //
            // "Future me" note: We've already checked for soul relocation. This
            //                   part only runs when that is enabled.
            if (d.config[BC::AllowSoulDisplacement] &&
                (d.config[BC::AllowPartiallyFillingSoulGems] ||
                 d.victim().soulSize() == SoulSize::Grand)) {
                LOG_TRACE("Looking up dual soul filled gems with a black soul");

                const bool result =
                    tryReplaceBlackSoulInDualSoulGemWithWhiteSoul_(d);

                if (result) {
                    d.notifySoulTrapSuccess(
                        SoulTrapSuccessMessage::SoulDisplaced,
                        d.victim());

                    return true;
                }
            }
        } else {
            // Without soul relocation, we need to minimize soul loss by
            // displacing the smallest soul first.
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
            for (SoulSizeValue containedSoulSize = SoulSize::None;
                 containedSoulSize < maxContainedSoulSizeToSearch;
                 ++containedSoulSize) {
                for (SoulGemCapacityValue capacity =
                         toSoulGemCapacity(d.victim().soulSize());
                     capacity <= maxSoulCapacityToSearch;
                     ++capacity) {
                    LOG_TRACE_FMT(
                        "Looking up white soul gems with capacity = {:t}, "
                        "containedSoulSize = {:t}",
                        capacity,
                        containedSoulSize);

                    const bool result = fillWhiteSoulGem_(
                        capacity,
                        containedSoulSize,
                        d.victim().soulSize(),
                        d);

                    if (result) {
                        // We've checked for soul relocation already. No need to
                        // do that again here.
                        if (containedSoulSize > SoulSize::None) {
                            d.notifySoulTrapSuccess(
                                SoulTrapSuccessMessage::SoulDisplaced,
                                d.victim());
                        } else {
                            d.notifySoulTrapSuccess(
                                SoulTrapSuccessMessage::SoulCaptured,
                                d.victim());
                        }

                        return true;
                    }
                }
            }
        }

        return false;
    }

    template <bool AllowSoulDisplacement>
    bool trapShrunkSoul_(SoulTrapData& d)
    {
        LOG_TRACE("Trapping shrunk white soul..."sv);

        const auto& soulGemMap = YASTMConfig::getInstance().soulGemMap();

        // Avoid shrinking a soul more than necessary. Any soul we displace must
        // be smaller than the soul gem capacity itself, and shrunk souls always
        // fully fill the soul gem. This suggests that we generally lose more
        // from shrinking the soul than losing a displaced soul.
        //
        // Because of this, we don't have special prioritization for when soul
        // relocation is disabled.
        //
        // This algorithm matches the one for trapping full white souls when
        // both displacement and relocation are enabled, except that we iterate
        // over soul capacity in descending order.

        for (SoulGemCapacityValue capacity =
                 toSoulGemCapacity(d.victim().soulSize()) - 1;
             capacity >= SoulGemCapacity::First;
             --capacity) {
            // When displacement is allowed, we search soul gems with contained
            // soul sizes up to one size lower than the incoming soul. Since the
            // incoming soul size varies depending on the shrunk soul size, we
            // put this inside the loop.
            //
            // If it's not allowed, we only look up empty soul gems.
            //
            // Note: Loop range is end-EXclusive, so we set this to
            // SoulSize::Petty as the next lowest soul size after
            // SoulSize::None.
            const SoulSize maxContainedSoulSizeToSearch =
                AllowSoulDisplacement ? toSoulSize(capacity) : SoulSize::Petty;

            for (SoulSizeValue containedSoulSize = SoulSize::None;
                 containedSoulSize < maxContainedSoulSizeToSearch;
                 ++containedSoulSize) {
                LOG_TRACE_FMT(
                    "Looking up white soul gems with capacity = {:t}, "
                    "containedSoulSize = {:t}",
                    capacity,
                    containedSoulSize);

                const auto& sourceSoulGems =
                    soulGemMap.getSoulGemsWith(capacity, containedSoulSize);

                const bool isFillSuccessful =
                    fillSoulGem_(sourceSoulGems, toSoulSize(capacity), d);

                if (isFillSuccessful) {
                    d.notifySoulTrapSuccess(
                        SoulTrapSuccessMessage::SoulShrunk,
                        d.victim());

                    if (d.config[BC::AllowSoulRelocation] &&
                        containedSoulSize > SoulSize::None) {
                        d.victims().emplace(
                            static_cast<SoulSize>(containedSoulSize));
                    }

                    return true;
                }
            }
        }

        return false;
    }

    bool trapShrunkSoul_(SoulTrapData& d)
    {
        return d.config[BC::AllowSoulDisplacement] ? trapShrunkSoul_<true>(d)
                                                   : trapShrunkSoul_<false>(d);
    }

    bool trapSplitSoul_(SoulTrapData& d)
    {
        LOG_TRACE("Trapping split white soul...");

        const auto& soulGemMap = YASTMConfig::getInstance().soulGemMap();

        // Don't look up non-empty soul gems if we can't displace souls.
        //
        // NOTE: Loop range is end-EXclusive.
        const SoulSize maxContainedSoulSizeToSearch =
            d.config[BC::AllowSoulDisplacement] ? d.victim().soulSize()
                                                : SoulSize::Petty;

        // This part is an optimized version of the soul shrinking process.
        //
        // Like soul shrinking, if soul splitting happens, we do not need to
        // search "upwards" (i.e. look up soul gems larger than the size of the
        // split soul) since souls are only split if the search for vacant soul
        // gems greater or equal to the current soul size fails.
        //
        // Unlike soul shrinking, when trapping a split soul fails, it can break
        // into two smaller souls. This is better handled by the victims queue,
        // so we do not handle the actual shrinking and just figure out if there
        // are any suitable soul gems for the *current* soul size.
        //
        // Also, the displayed notification messages are different so we handle
        // this in a different function.
        for (SoulSizeValue containedSoulSize = SoulSize::None;
             containedSoulSize < maxContainedSoulSizeToSearch;
             ++containedSoulSize) {
            LOG_TRACE_FMT(
                "Looking up white soul gems with capacity = {:t}, "
                "containedSoulSize = {:t}",
                d.victim().soulSize(),
                containedSoulSize);

            const auto& sourceSoulGems = soulGemMap.getSoulGemsWith(
                toSoulGemCapacity(d.victim().soulSize()),
                containedSoulSize);

            const bool result =
                fillSoulGem_(sourceSoulGems, d.victim().soulSize(), d);

            if (result) {
                d.notifySoulTrapSuccess(
                    SoulTrapSuccessMessage::SoulSplit,
                    d.victim());

                if (d.config[BC::AllowSoulRelocation] &&
                    containedSoulSize > SoulSize::None) {
                    d.victims().emplace(
                        static_cast<SoulSize>(containedSoulSize));
                }

                return true;
            }
        }

        return false;
    }

    void splitSoul_(const Victim& victim, VictimsQueue& victimQueue)
    {
        // Raw Soul Sizes:
        // - Grand   = 3000 = Greater + Common
        // - Greater = 2000 = Common + Common
        // - Common  = 1000 = Lesser + Lesser
        // - Lesser  = 500  = Petty + Petty
        // - Petty   = 250
        switch (victim.soulSize()) {
        // Do not split black souls.
        // case SoulSize::Black:
        case SoulSize::Grand:
            victimQueue.emplace(victim.actor(), SoulSize::Greater, true);
            victimQueue.emplace(victim.actor(), SoulSize::Common, true);
            break;
        case SoulSize::Greater:
            victimQueue.emplace(victim.actor(), SoulSize::Common, true);
            victimQueue.emplace(victim.actor(), SoulSize::Common, true);
            break;
        case SoulSize::Common:
            victimQueue.emplace(victim.actor(), SoulSize::Lesser, true);
            victimQueue.emplace(victim.actor(), SoulSize::Lesser, true);
            break;
        case SoulSize::Lesser:
            victimQueue.emplace(victim.actor(), SoulSize::Petty, true);
            victimQueue.emplace(victim.actor(), SoulSize::Petty, true);
            break;
        }
    }

    std::mutex trapSoulMutex_; /* Process only one soul trap at a time. */
} // namespace

bool trapSoul(RE::Actor* const caster, RE::Actor* const victim)
{
    if (caster == nullptr) {
        LOG_TRACE("Caster is null.");
        return false;
    }

    if (victim == nullptr) {
        LOG_TRACE("Victim is null.");
        return false;
    }

    if (caster->IsDead(false)) {
        LOG_TRACE("Caster is dead.");
        return false;
    }

    if (!victim->IsDead(false)) {
        LOG_TRACE("Victim is not dead.");
        return false;
    }

    // We begin the mutex here since we're checking isSoulTrapped status next.
    std::lock_guard<std::mutex> guard(trapSoulMutex_);

    if (native::getRemainingSoulLevelValue(victim) == SoulLevelValue::None) {
        LOG_TRACE("Victim has already been soul trapped.");
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
        SoulTrapData d(caster);

        switch (d.config.get<EC::SoulTrapLevelingType>()) {
        case SoulTrapLevelingType::Degradation:
            {
                const auto maxSoulSize = d.maxTrappableSoulSize();
                LOG_TRACE_FMT("Max trappable soul size: {:tu}", maxSoulSize);

                if (maxSoulSize == SoulSize::None) {
                    LOG_TRACE(
                        "Caster conjuration level is too low for any soul "
                        "trap.");
                    d.notifySoulTrapFailure(SoulTrapFailureMessage::SoulLost);
                    return false;
                }

                auto victimSoulSize = getActorSoulSize(victim);
                LOG_TRACE_FMT("Victim's soul size: {:tu}", maxSoulSize);

                // Black souls can't be degraded. Reject entirely.
                if (victimSoulSize == SoulSize::Black &&
                    maxSoulSize < SoulSize::Black) {
                    LOG_TRACE(
                        "Caster conjuration level is too low to trap black "
                        "souls.");
                    d.notifySoulTrapFailure(SoulTrapFailureMessage::SoulLost);
                    return false;
                }

                if (victimSoulSize > maxSoulSize) {
                    LOG_TRACE_FMT("Degraded soul size: {}", maxSoulSize);
                    d.victims().emplace(victim, maxSoulSize, false);
                    d.setDegradedSoulTrap();
                } else {
                    d.victims().emplace(victim);
                }
                break;
            }
        case SoulTrapLevelingType::Loss:
            {
                const auto victimSoulSize = getActorSoulSize(victim);
                LOG_TRACE_FMT("Victim's soul size: {:tu}", victimSoulSize);

                const auto levelThreshold =
                    d.getThresholdForSoulSize(victimSoulSize);
                LOG_TRACE_FMT("Threshold level for victim: {}", levelThreshold);
                LOG_TRACE_FMT("Caster soul trap level: {}", d.soulTrapLevel());

                if (d.soulTrapLevel() < levelThreshold) {
                    const auto scaling =
                        d.config[IC::SoulLossSuccessChanceScaling] / 100.0;

                    double chanceThreshold;

                    if (d.config[BC::AllowSoulLossProgression]) {
                        chanceThreshold =
                            (d.soulTrapLevel() * scaling) / levelThreshold;
                    } else {
                        chanceThreshold = scaling;
                    }

                    const auto x = Rng::getInstance().generateUniform(0.0, 1.0);

                    LOG_TRACE_FMT("chance={}, x={}", chanceThreshold, x);

                    if (chanceThreshold < x) {
                        LOG_TRACE("Soul lost.");
                        d.notifySoulTrapFailure(
                            SoulTrapFailureMessage::SoulLost);
                        return false;
                    }
                }

                d.victims().emplace(victim);
                break;
            }
        default:
            d.victims().emplace(victim);
            break;
        }

        while (!d.victims().empty()) {
            d.updateLoopVariables();

            LOG_TRACE_FMT("Processing soul trap victim: {}", d.victim());

            if (d.casterInventoryStatus() !=
                InventoryStatus::HasSoulGemsToFill) {
                // Caster doesn't have any soul gems. Stop looking.
                LOG_TRACE("Caster has no soul gems to fill. Stop looking.");
                break;
            }

            if (d.victim().soulSize() == SoulSize::Black) {
                if (trapBlackSoul_(d)) {
                    isSoulTrapSuccessful = true;
                    continue; // Process next soul.
                }
            } else if (d.victim().isSplitSoul()) {
                assert(
                    d.config.get<EC::SoulShrinkingTechnique>() ==
                    SoulShrinkingTechnique::Split);

                if (trapSplitSoul_(d)) {
                    isSoulTrapSuccessful = true;
                    continue; // Process next soul.
                }

                splitSoul_(d.victim(), d.victims());
                continue; // Process next soul.
            } else {
                if (trapFullSoul_(d)) {
                    isSoulTrapSuccessful = true;
                    continue; // Process next soul.
                }

                // If we've reached this point, we start reducing the size of
                // the soul.
                //
                // Standard soul shrinking is prioritized over soul splitting.
                // Enabling both will implicitly turn off soul splitting.
                const auto soulShrinkingTechnique =
                    d.config.get<EC::SoulShrinkingTechnique>();

                if (soulShrinkingTechnique == SoulShrinkingTechnique::Shrink) {
                    if (trapShrunkSoul_(d)) {
                        isSoulTrapSuccessful = true;
                        continue; // Process next soul.
                    }
                } else if (
                    soulShrinkingTechnique == SoulShrinkingTechnique::Split) {
                    splitSoul_(d.victim(), d.victims());
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
                    LOG_TRACE("Flagging soul trapped victim...");
                    process->middleHigh->soulTrapped = true;
                }
            }
        } else {
            // Shorten it so we can keep it in one line after formatting for
            // readability.
            using Message = SoulTrapFailureMessage;

            switch (d.casterInventoryStatus()) {
            case InventoryStatus::AllSoulGemsFilled:
                d.notifySoulTrapFailure(Message::AllSoulGemsFilled);
                break;
            case InventoryStatus::NoSoulGemsOwned:
                d.notifySoulTrapFailure(Message::NoSoulGemsOwned);
                break;
            default:
                if (d.config.get<EC::SoulShrinkingTechnique>() !=
                    SoulShrinkingTechnique::None) {
                    d.notifySoulTrapFailure(Message::NoSuitableSoulGem);
                } else {
                    d.notifySoulTrapFailure(Message::NoSoulGemLargeEnough);
                }
            }
        }

        return isSoulTrapSuccessful;
    } catch (const std::exception& error) {
        printError(error);
    }

    return false;
}
