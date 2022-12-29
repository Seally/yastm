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
#include "messages.hpp"
#include "Victim.hpp"
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
    InventoryStatus casterInventoryStatus_;
    UnorderedInventoryItemMap inventoryMap_;

    VictimsQueue victims_;
    std::optional<Victim> victim_;

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
    InventoryStatus casterInventoryStatus() const;
    const InventoryItemMap& inventoryMap() const;

    VictimsQueue& victims() noexcept { return victims_; }
    const VictimsQueue& victims() const noexcept { return victims_; }

    const Victim& victim() const { return victim_.value(); }

    void notifySoulTrapFailure(const SoulTrapFailureMessage message);

    void notifySoulTrapSuccess(
        const SoulTrapSuccessMessage message,
        const Victim& victim);
};

inline SoulTrapData::SoulTrapData(RE::Actor* const caster)
    : caster_(caster)
    , config(YASTMConfig::getInstance())
{}

template <typename MessageKey>
inline void SoulTrapData::notify_(const MessageKey message)
{
    if (notifyCount_ < MAX_NOTIFICATION_COUNT &&
        config[BC::AllowNotifications]) {
        RE::DebugNotification(getMessage(message));
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
