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

#include "RE/S/SoulsTrapped.hpp"

#include "global.hpp"
#include "messages.hpp"
#include "Victim.hpp"
#include "config/YASTMConfig.hpp"
#include "formatters/TESSoulGem.hpp"
#include "utilities/printerror.hpp"
#include "utilities/TESObjectREFR.hpp"
#include "utilities/TESSoulGem.hpp"
#include "utilities/Timer.hpp"

using namespace std::literals;

using VictimsQueue = std::priority_queue<Victim, std::deque<Victim>>;
/**
 * @brief Boolean Config Key
 */
using BC = BoolConfigKey;
/**
 * @brief Enum Config Key
 */
using EC = EnumConfigKey;

namespace native {
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

enum class _InventoryStatus {
    HasSoulGemsToFill,
    /**
     * @brief Caster does not own any soul gems.
     */
    NoSoulGemsOwned,
    /**
     * @brief Caster has soul gems, but all are fully-filled. 
    */
    AllSoulGemsFilled,
};

/**
 * @brief Stores and bookkeeps the data for various soul trap variables so we
 * don't end up with functions needing half a dozen arguments.
 */
class _SoulTrapData {
    static const std::size_t MAX_NOTIFICATION_COUNT = 1;
    std::size_t _notifyCount = 0;
    bool _isStatIncremented = false;
    bool _isInventoryMapDirty = true;

    RE::Actor* _caster;
    _InventoryStatus _casterInventoryStatus;
    UnorderedInventoryItemMap _inventoryMap;

    VictimsQueue _victims;
    std::optional<Victim> _victim;

    template <typename MessageKey>
    void _notify(const MessageKey message)
    {
        if (_notifyCount < MAX_NOTIFICATION_COUNT &&
            config[BC::AllowNotifications]) {
            RE::DebugNotification(getMessage(message));
            ++_notifyCount;
        }
    }

    void _incrementSoulsTrappedStat(RE::Actor* const victim)
    {
        if (!_isStatIncremented) {
            RE::SoulsTrapped::SendEvent(caster(), victim);
            _isStatIncremented = true;
        }
    }

public:
    using InventoryItemMap = UnorderedInventoryItemMap;

    const YASTMConfig::Snapshot config;

    _SoulTrapData(RE::Actor* const caster)
        : _caster{caster}
        , config{YASTMConfig::getInstance()}
    {
        if (config[BC::AllowSoulDiversion] &&
            config[BC::PerformSoulDiversionInDLL] && !caster->IsPlayerRef() &&
            caster->IsPlayerTeammate()) {
            const auto playerActor = _SoulTrapData::player();

            if (playerActor != nullptr) {
                _caster = playerActor;

                LOG_TRACE("Soul trap diverted to player."sv);
            } else {
                LOG_WARN("Failed to find player reference for soul diversion.");
            }
        }
    }

    _SoulTrapData(const _SoulTrapData&) = delete;
    _SoulTrapData(_SoulTrapData&&) = delete;
    _SoulTrapData& operator=(const _SoulTrapData&) = delete;
    _SoulTrapData& operator=(_SoulTrapData&&) = delete;

    static RE::Actor* player()
    {
        // Player base form ID: 0x00000007
        // Player ref form ID:  0x00000014
        return RE::TESForm::LookupByID<RE::Actor>(0x14);
    }

    void setInventoryHasChanged() { _isInventoryMapDirty = true; }

    void updateLoopVariables()
    {
        _victim.emplace(_victims.top());
        _victims.pop();

        if (_isInventoryMapDirty) {
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

                if (soulGem->GetMaximumCapacity() ==
                    soulGem->GetContainedSoul()) {
                    ++maxFilledSoulGemsCount;
                }
            }

            if (_inventoryMap.size() <= 0) {
                _casterInventoryStatus = _InventoryStatus::NoSoulGemsOwned;
            } else if (_inventoryMap.size() == maxFilledSoulGemsCount) {
                _casterInventoryStatus = _InventoryStatus::AllSoulGemsFilled;
            } else {
                _casterInventoryStatus = _InventoryStatus::HasSoulGemsToFill;
            }

            _isInventoryMapDirty = false;
        }
    }

    RE::Actor* caster() const { return _caster; }
    _InventoryStatus casterInventoryStatus() const
    {
        // This should not happen if the class is used correctly (the class does
        // not manage these resources on its own for performance).
        assert(!_isInventoryMapDirty);
        return _casterInventoryStatus;
    }
    const InventoryItemMap& inventoryMap() const
    {
        // This should not happen if the class is used correctly (the class does
        // not manage these resources on its own for performance).
        assert(!_isInventoryMapDirty);
        return _inventoryMap;
    }

    VictimsQueue& victims() { return _victims; }
    const VictimsQueue& victims() const { return _victims; }

    const Victim& victim() const { return _victim.value(); }

    void notifySoulTrapFailure(const SoulTrapFailureMessage message)
    {
        if (_caster->IsPlayerRef()) {
            _notify(message);
        }
    }

    void notifySoulTrapSuccess(
        const SoulTrapSuccessMessage message,
        const Victim& victim)
    {
        if (_caster->IsPlayerRef() && victim.isPrimarySoul()) {
            _notify(message);
            _incrementSoulsTrappedStat(victim.actor());
        }
    }
};

struct _SearchResult {
    const std::size_t index;
    const RE::TESObjectREFR::Count itemCount;
    RE::InventoryEntryData* entryData;
};

std::optional<_SearchResult> _findFirstOwnedObjectInList(
    const _SoulTrapData::InventoryItemMap& inventoryMap,
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

    if (d.config[BC::AllowExtraSoulRelocation] ||
        d.config[BC::PreserveOwnership]) {
        oldExtraList = _getFirstExtraDataList(soulGemToRemoveEntryData);
    }

    if (d.config[BC::AllowExtraSoulRelocation] && oldExtraList != nullptr) {
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
            d.victims().emplace(soulSize);
        }
    }

    if (d.config[BC::PreserveOwnership]) {
        newExtraList = _createExtraDataListFromOriginal(oldExtraList);
    }

    LOG_TRACE_FMT(
        "Replacing soul gems in {}'s inventory"sv,
        d.caster()->GetName());
    LOG_TRACE_FMT("- from: {}"sv, *soulGemToRemove);
    LOG_TRACE_FMT("- to: {}"sv, *soulGemToAdd);

    d.caster()->AddObjectToContainer(soulGemToAdd, newExtraList, 1, nullptr);
    d.caster()->RemoveItem(
        soulGemToRemove,
        1,
        RE::ITEM_REMOVE_REASON::kRemove,
        oldExtraList,
        nullptr);
    d.setInventoryHasChanged();
}

bool _fillSoulGem(
    const std::vector<RE::TESSoulGem*>& sourceSoulGems,
    const std::vector<RE::TESSoulGem*>& targetSoulGems,
    _SoulTrapData& d)
{
    const auto maybeFirstOwned =
        _findFirstOwnedObjectInList(d.inventoryMap(), sourceSoulGems);

    if (maybeFirstOwned.has_value()) {
        const auto& firstOwned = maybeFirstOwned.value();

        const auto soulGemToAdd = targetSoulGems[firstOwned.index];
        const auto soulGemToRemove = sourceSoulGems[firstOwned.index];

        _replaceSoulGem(soulGemToAdd, soulGemToRemove, firstOwned.entryData, d);

        return true;
    }

    return false;
}

bool _fillWhiteSoulGem(
    const SoulSize capacity,
    const SoulSize sourceContainedSoulSize,
    const SoulSize targetContainedSoulSize,
    _SoulTrapData& d)
{
    const auto& soulGemMap = YASTMConfig::getInstance().soulGemMap();

    const auto& sourceSoulGems =
        soulGemMap.getWhiteSoulGemsWith(capacity, sourceContainedSoulSize);
    const auto& targetSoulGems =
        soulGemMap.getWhiteSoulGemsWith(capacity, targetContainedSoulSize);

    return _fillSoulGem(sourceSoulGems, targetSoulGems, d);
}

bool _fillBlackSoulGem(_SoulTrapData& d)
{
    const auto& soulGemMap = YASTMConfig::getInstance().soulGemMap();

    const auto& sourceSoulGems =
        soulGemMap.getPureBlackSoulGemsWith(SoulSize::None);
    const auto& targetSoulGems =
        soulGemMap.getPureBlackSoulGemsWith(SoulSize::Black);

    return _fillSoulGem(sourceSoulGems, targetSoulGems, d);
}

bool _tryReplaceBlackSoulInDualSoulGemWithWhiteSoul(_SoulTrapData& d)
{
    const auto& soulGemMap = YASTMConfig::getInstance().soulGemMap();

    // Find our black-filled dual soul gem.
    const auto& sourceSoulGems =
        soulGemMap.getWhiteSoulGemsWith(SoulSize::Black, SoulSize::Black);
    const auto maybeFirstOwned =
        _findFirstOwnedObjectInList(d.inventoryMap(), sourceSoulGems);

    // If the black-filled dual soul exists in the inventory and we can fill an
    // empty pure black soul gem, fill the dual soul gem with our white soul.
    if (maybeFirstOwned.has_value() && _fillBlackSoulGem(d)) {
        const auto& targetSoulGems = soulGemMap.getWhiteSoulGemsWith(
            SoulSize::Black,
            d.victim().soulSize());

        const auto& firstOwned = maybeFirstOwned.value();

        const auto soulGemToAdd = targetSoulGems[firstOwned.index];
        const auto soulGemToRemove = sourceSoulGems[firstOwned.index];

        _replaceSoulGem(soulGemToAdd, soulGemToRemove, firstOwned.entryData, d);

        return true;
    }

    return false;
}

bool _trapBlackSoul(_SoulTrapData& d)
{
    LOG_TRACE("Trapping black soul..."sv);

    LOG_TRACE("Looking up pure empty black soul gems"sv);
    const bool isSoulTrapped = _fillBlackSoulGem(d);

    if (isSoulTrapped) {
        d.notifySoulTrapSuccess(
            SoulTrapSuccessMessage::SoulCaptured,
            d.victim());

        return true;
    }

    const auto& soulGemMap = YASTMConfig::getInstance().soulGemMap();

    // When displacement is allowed, we search dual soul gems with a contained
    // soul size up to SoulSize::Grand to allow displacing white grand souls.
    //
    // Note: Loop range is end-EXclusive.
    const SoulSize maxContainedSoulSizeToSearch =
        d.config[BC::AllowSoulDisplacement] ? SoulSize::Black : SoulSize::Petty;

    const auto& targetSoulGems =
        soulGemMap.getWhiteSoulGemsWith(SoulSize::Black, SoulSize::Black);

    for (std::size_t containedSoulSize = 0;
         containedSoulSize < maxContainedSoulSizeToSearch;
         ++containedSoulSize) {
        LOG_TRACE_FMT(
            "Looking up dual soul gems with containedSoulSize = {}"sv,
            containedSoulSize);

        const auto& sourceSoulGems = soulGemMap.getWhiteSoulGemsWith(
            SoulSize::Black,
            static_cast<SoulSize>(containedSoulSize));

        const bool result = _fillSoulGem(sourceSoulGems, targetSoulGems, d);

        if (result) {
            if (d.config[BC::AllowSoulRelocation] &&
                containedSoulSize > SoulSize::None) {
                d.notifySoulTrapSuccess(
                    SoulTrapSuccessMessage::SoulDisplaced,
                    d.victim());
                d.victims().emplace(static_cast<SoulSize>(containedSoulSize));
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

bool _trapFullSoul(_SoulTrapData& d)
{
    LOG_TRACE("Trapping full white soul..."sv);

    const auto& soulGemMap = YASTMConfig::getInstance().soulGemMap();

    // When partial trapping is allowed, we search all soul sizes up to
    // Grand. If it's not allowed, we only look at soul gems with the same
    // soul size.
    //
    // Note: Loop range is end-INclusive.
    const SoulSize maxSoulCapacityToSearch =
        d.config[BC::AllowPartiallyFillingSoulGems] ? SoulSize::Black
                                                    : d.victim().soulSize();

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
        for (std::size_t capacity =
                 static_cast<std::size_t>(d.victim().soulSize());
             capacity <= maxSoulCapacityToSearch;
             ++capacity) {
            const auto& targetSoulGems = soulGemMap.getWhiteSoulGemsWith(
                static_cast<SoulSize>(capacity),
                d.victim().soulSize());

            for (std::size_t containedSoulSize =
                     static_cast<std::size_t>(SoulSize::None);
                 containedSoulSize < maxContainedSoulSizeToSearch;
                 ++containedSoulSize) {
                LOG_TRACE_FMT(
                    "Looking up white soul gems with capacity = {}, containedSoulSize = {}"sv,
                    capacity,
                    containedSoulSize);

                const auto& sourceSoulGems = soulGemMap.getWhiteSoulGemsWith(
                    static_cast<SoulSize>(capacity),
                    static_cast<SoulSize>(containedSoulSize));

                const bool result =
                    _fillSoulGem(sourceSoulGems, targetSoulGems, d);

                if (result) {
                    // We've checked for soul relocation already. No need to do
                    // that again here.
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
        // any exists, check if there is an empty pure black soul gem and fill
        // it, then fill the dual soul gem with the new soul.
        //
        // This is handled without using the victims queue to avoid an infinite
        // loop from black souls displacing white souls and white souls
        // displacing black souls.
        //
        // "Future me" note: We've already checked for soul relocation.
        if (d.config[BC::AllowSoulDisplacement] &&
            (d.config[BC::AllowPartiallyFillingSoulGems] ||
             d.victim().soulSize() == SoulSize::Grand)) {
            LOG_TRACE("Looking up dual soul filled gems with a black soul"sv);

            const bool result =
                _tryReplaceBlackSoulInDualSoulGemWithWhiteSoul(d);

            if (result) {
                d.notifySoulTrapSuccess(
                    SoulTrapSuccessMessage::SoulDisplaced,
                    d.victim());

                return true;
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
        for (std::size_t containedSoulSize =
                 static_cast<std::size_t>(SoulSize::None);
             containedSoulSize < maxContainedSoulSizeToSearch;
             ++containedSoulSize) {
            for (std::size_t capacity =
                     static_cast<std::size_t>(d.victim().soulSize());
                 capacity <= maxSoulCapacityToSearch;
                 ++capacity) {
                LOG_TRACE_FMT(
                    "Looking up white soul gems with capacity = {}, containedSoulSize = {}"sv,
                    capacity,
                    containedSoulSize);

                const bool result = _fillWhiteSoulGem(
                    static_cast<SoulSize>(capacity),
                    static_cast<SoulSize>(containedSoulSize),
                    d.victim().soulSize(),
                    d);

                if (result) {
                    // We've checked for soul relocation already. No need to do
                    // that again here.
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
bool _trapShrunkSoul(_SoulTrapData& d)
{
    LOG_TRACE("Trapping shrunk white soul..."sv);

    const auto& soulGemMap = YASTMConfig::getInstance().soulGemMap();

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

    // Use a signed int instead of std::size_t to prevent a possible underflow
    // issue since we're counting downwards.
    for (int capacity = d.victim().soulSize() - 1; capacity > SoulSize::None;
         --capacity) {
        const auto& targetSoulGems = soulGemMap.getWhiteSoulGemsWith(
            static_cast<SoulSize>(capacity),
            static_cast<SoulSize>(capacity));

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
            AllowSoulDisplacement ? static_cast<SoulSize>(capacity)
                                  : SoulSize::Petty;

        for (std::size_t containedSoulSize =
                 static_cast<std::size_t>(SoulSize::None);
             containedSoulSize < maxContainedSoulSizeToSearch;
             ++containedSoulSize) {
            LOG_TRACE_FMT(
                "Looking up white soul gems with capacity = {}, containedSoulSize = {}"sv,
                capacity,
                containedSoulSize);

            const auto& sourceSoulGems = soulGemMap.getWhiteSoulGemsWith(
                static_cast<SoulSize>(capacity),
                static_cast<SoulSize>(containedSoulSize));

            const bool isFillSuccessful =
                _fillSoulGem(sourceSoulGems, targetSoulGems, d);

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

bool _trapShrunkSoul(_SoulTrapData& d)
{
    return d.config[BC::AllowSoulDisplacement] ? _trapShrunkSoul<true>(d)
                                               : _trapShrunkSoul<false>(d);
}

bool _trapSplitSoul(_SoulTrapData& d)
{
    LOG_TRACE("Trapping split white soul..."sv);

    const auto& soulGemMap = YASTMConfig::getInstance().soulGemMap();

    const SoulSize maxContainedSoulSizeToSearch =
        d.config[BC::AllowSoulDisplacement]
            ? static_cast<SoulSize>(d.victim().soulSize())
            : SoulSize::Petty;

    const auto& targetSoulGems = soulGemMap.getWhiteSoulGemsWith(
        static_cast<SoulSize>(d.victim().soulSize()),
        static_cast<SoulSize>(d.victim().soulSize()));

    for (std::size_t containedSoulSize =
             static_cast<std::size_t>(SoulSize::None);
         containedSoulSize < maxContainedSoulSizeToSearch;
         ++containedSoulSize) {
        LOG_TRACE_FMT(
            "Looking up white soul gems with capacity = {}, containedSoulSize = {}"sv,
            d.victim().soulSize(),
            containedSoulSize);

        const auto& sourceSoulGems = soulGemMap.getWhiteSoulGemsWith(
            static_cast<SoulSize>(d.victim().soulSize()),
            static_cast<SoulSize>(containedSoulSize));

        const bool result = _fillSoulGem(sourceSoulGems, targetSoulGems, d);

        if (result) {
            d.notifySoulTrapSuccess(
                SoulTrapSuccessMessage::SoulSplit,
                d.victim());

            if (d.config[BC::AllowSoulRelocation] &&
                containedSoulSize > SoulSize::None) {
                d.victims().emplace(static_cast<SoulSize>(containedSoulSize));
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
    // Do not split black souls.
    // case SoulSize::Black:
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
                BoolConfigKey::AllowProfiling)) {
            LOG_INFO_FMT("Time to trap soul: {:.7f} seconds", elapsedTime);
            RE::DebugNotification(
                fmt::format(
                    getMessage(MiscMessage::TimeTakenToTrapSoul),
                    elapsedTime)
                    .c_str());
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

        d.victims().emplace(victim);

#ifndef NDEBUG
        LOG_TRACE("Found configuration:"sv);

        forEachBoolConfigKey([&](const BoolConfigKey key) {
            LOG_TRACE_FMT("- {}: {}"sv, key, d.config[key]);
        });

        LOG_TRACE_FMT(
            "- {}: {}"sv,
            EC::SoulShrinkingTechnique,
            d.config.get<EC::SoulShrinkingTechnique>());
#endif // NDEBUG

        while (!d.victims().empty()) {
            d.updateLoopVariables();

            LOG_TRACE_FMT("Processing soul trap victim: {}", d.victim());

            if (d.casterInventoryStatus() !=
                _InventoryStatus::HasSoulGemsToFill) {
                // Caster doesn't have any soul gems. Stop looking.
                LOG_TRACE("Caster has no soul gems to fill. Stop looking."sv);
                break;
            }

            if (d.victim().soulSize() == SoulSize::Black) {
                if (_trapBlackSoul(d)) {
                    isSoulTrapSuccessful = true;
                    continue; // Process next soul.
                }
            } else if (d.victim().isSplitSoul()) {
                assert(
                    d.config.get<EC::SoulShrinkingTechnique>() ==
                    SoulShrinkingTechnique::Split);

                if (_trapSplitSoul(d)) {
                    isSoulTrapSuccessful = true;
                    continue; // Process next soul.
                }

                _splitSoul(d.victim(), d.victims());
                continue; // Process next soul.
            } else {
                if (_trapFullSoul(d)) {
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
                    if (_trapShrunkSoul(d)) {
                        isSoulTrapSuccessful = true;
                        continue; // Process next soul.
                    }
                } else if (
                    soulShrinkingTechnique == SoulShrinkingTechnique::Split) {
                    _splitSoul(d.victim(), d.victims());
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
            // Shorten it so we can keep it in one line after formatting for
            // readability.
            using Message = SoulTrapFailureMessage;

            switch (d.casterInventoryStatus()) {
            case _InventoryStatus::AllSoulGemsFilled:
                d.notifySoulTrapFailure(Message::AllSoulGemsFilled);
                break;
            case _InventoryStatus::NoSoulGemsOwned:
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

void _handleMessage(SKSE::MessagingInterface::Message* message)
{
    using namespace std::literals;

    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        YASTMConfig::getInstance().loadGameForms(
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
        // .text:0000000140634917
        0x48, 0x89, 0x58, 0x10,            // mov  [rax+10h], rbx
        0x48, 0x89, 0x68, 0x18,            // mov  [rax+18h], rbp
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
        config.checkDllDependencies(loadInterface);
        config.readConfigs();
    } catch (const std::exception& error) {
        printError(error);
        LOG_ERROR("Not installing trapSoul patch.");

        return false;
    }

    auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener(_handleMessage);

    const REL::ID soulTrap1_id{37863};
    constexpr std::uintptr_t callOffset = 0x17;
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
