#pragma once

#include <optional>

#include <RE/A/Actor.h>
#include <RE/M/Misc.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/S/SoulsTrapped.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESBoundObject.h>

#include "types.hpp"
#include "InventoryStatus.hpp"
#include "Victim.hpp"
#include "../global.hpp"
#include "../messages.hpp"
#include "../config/YASTMConfig.hpp"
#include "../utilities/misc.hpp"

/**
 * @brief Stores and bookkeeps the data for various soul trap variables so
 * we don't end up with functions needing half a dozen arguments.
 */
class SoulTrapData {
public:
    using InventoryItemMap = UnorderedInventoryItemMap;

private:
    static const std::size_t MAX_NOTIFICATION_COUNT = 1;
    std::size_t notifyCount_ = 0;
    bool isSoulTrapEventSent_ = false;
    bool isInventoryMapDirty_ = true;

    RE::Actor* caster_;
    // [DEVNOTE] Make sure this variable appears before the config variable
    //           since the value is passed to the snapshot's constructor.
    /**
     * @brief The "level" of the soul trap. This is currently based on the
     * caster's conjuration skill level.
     *
     * Note: This is not necessarily the player level.
     */
    int soulTrapLevel_;
    SoulSize maxTrappableSoulSize_;
    InventoryStatus casterInventoryStatus_;
    UnorderedInventoryItemMap inventoryMap_;

    VictimsQueue victims_;
    std::optional<Victim> victim_;
    bool isDegradedSoulTrap_ = false;

    template <typename MessageKey>
    void notify_(MessageKey message);
    void sendSoulTrapEvent_(RE::Actor* victim);
    void resetInventoryData_();

public:
    const YASTMConfig::Snapshot config;
    SoulTrapData(RE::Actor* caster);

    SoulTrapData(const SoulTrapData&) = delete;
    SoulTrapData(SoulTrapData&&) = delete;
    SoulTrapData& operator=(const SoulTrapData&) = delete;
    SoulTrapData& operator=(SoulTrapData&&) = delete;

    void setInventoryHasChanged() noexcept { isInventoryMapDirty_ = true; }
    void updateLoopVariables();

    RE::Actor* caster() const noexcept { return caster_; }
    int soulTrapLevel() const noexcept { return soulTrapLevel_; }
    SoulSize maxTrappableSoulSize() const noexcept
    {
        return maxTrappableSoulSize_;
    }
    int getThresholdForSoulSize(SoulSize soulSize) const;
    InventoryStatus casterInventoryStatus() const;
    const InventoryItemMap& inventoryMap() const;

    VictimsQueue& victims() noexcept { return victims_; }
    const VictimsQueue& victims() const noexcept { return victims_; }

    const Victim& victim() const { return victim_.value(); }
    void setDegradedSoulTrap(bool isDegraded = true)
    {
        isDegradedSoulTrap_ = isDegraded;
    }
    bool isDegradedSoulTrap() const { return isDegradedSoulTrap_; }

    void notifySoulTrapFailure(const SoulTrapFailureMessage message);

    void notifySoulTrapSuccess(
        const SoulTrapSuccessMessage message,
        const Victim& victim);
};

template <typename MessageKey>
inline void SoulTrapData::notify_(const MessageKey message)
{
    if (notifyCount_ < MAX_NOTIFICATION_COUNT &&
        config[BC::AllowNotifications]) {
        RE::DebugNotification(getMessage(message));
        ++notifyCount_;
    }
}

template <>
inline void SoulTrapData::notify_<SoulTrapSuccessMessage>(
    const SoulTrapSuccessMessage message)
{
    if (notifyCount_ < MAX_NOTIFICATION_COUNT &&
        config[BC::AllowNotifications]) {
        RE::DebugNotification(getMessage(message, isDegradedSoulTrap()));
        ++notifyCount_;
    }
}

inline void SoulTrapData::sendSoulTrapEvent_(RE::Actor* const victim)
{
    if (!isSoulTrapEventSent_) {
        RE::SoulsTrapped::SendEvent(caster(), victim);
        isSoulTrapEventSent_ = true;
    }
}

inline void SoulTrapData::updateLoopVariables()
{
    victim_.emplace(victims_.top());
    victims_.pop();

    if (isInventoryMapDirty_) {
        resetInventoryData_();
    }
}

inline int SoulTrapData::getThresholdForSoulSize(const SoulSize soulSize) const
{
    using IC = IntConfigKey;

    switch (soulSize) {
    case SoulSize::Black:
        return config[IC::SoulTrapThresholdBlack];
    case SoulSize::Grand:
        return config[IC::SoulTrapThresholdGrand];
    case SoulSize::Greater:
        return config[IC::SoulTrapThresholdGreater];
    case SoulSize::Common:
        return config[IC::SoulTrapThresholdCommon];
    case SoulSize::Lesser:
        return config[IC::SoulTrapThresholdLesser];
    case SoulSize::Petty:
        return config[IC::SoulTrapThresholdPetty];
    }

    return 1;
}

inline InventoryStatus SoulTrapData::casterInventoryStatus() const
{
    // This should not happen if the class is used correctly (the class does
    // not manage these resources on its own for performance).
    assert(!isInventoryMapDirty_);
    return casterInventoryStatus_;
}

inline const SoulTrapData::InventoryItemMap& SoulTrapData::inventoryMap() const
{
    // This should not happen if the class is used correctly (the class does
    // not manage these resources on its own for performance).
    assert(!isInventoryMapDirty_);
    return inventoryMap_;
}

inline void
    SoulTrapData::notifySoulTrapFailure(const SoulTrapFailureMessage message)
{
    if (caster_->IsPlayerRef()) {
        notify_(message);
    }
}

inline void SoulTrapData::notifySoulTrapSuccess(
    const SoulTrapSuccessMessage message,
    const Victim& victim)
{
    if (caster_->IsPlayerRef() && victim.isPrimarySoul()) {
        notify_(message);
        sendSoulTrapEvent_(victim.actor());
    }
}
