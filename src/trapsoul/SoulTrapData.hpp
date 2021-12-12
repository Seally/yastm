#pragma once

#include <optional>

#include <RE/A/Actor.h>
#include <RE/M/Misc.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESBoundObject.h>

#include "types.hpp"
#include "InventoryStatus.hpp"
#include "messages.hpp"
#include "Victim.hpp"
#include "../RE/S/SoulsTrapped.hpp"
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
    std::size_t _notifyCount = 0;
    bool _isSoulTrapEventSent = false;
    bool _isInventoryMapDirty = true;

    RE::Actor* _caster;
    InventoryStatus _casterInventoryStatus;
    UnorderedInventoryItemMap _inventoryMap;

    VictimsQueue _victims;
    std::optional<Victim> _victim;

    template <typename MessageKey>
    void _notify(const MessageKey message);
    void _sendSoulTrapEvent(RE::Actor* const victim);
    void _resetInventoryData();

public:
    const YASTMConfig::Snapshot config;

    SoulTrapData(RE::Actor* const caster);

    SoulTrapData(const SoulTrapData&) = delete;
    SoulTrapData(SoulTrapData&&) = delete;
    SoulTrapData& operator=(const SoulTrapData&) = delete;
    SoulTrapData& operator=(SoulTrapData&&) = delete;

    void setInventoryHasChanged() { _isInventoryMapDirty = true; }

    void updateLoopVariables();

    RE::Actor* caster() const { return _caster; }
    InventoryStatus casterInventoryStatus() const;
    const InventoryItemMap& inventoryMap() const;

    VictimsQueue& victims() { return _victims; }
    const VictimsQueue& victims() const { return _victims; }

    const Victim& victim() const { return _victim.value(); }

    void notifySoulTrapFailure(const SoulTrapFailureMessage message);

    void notifySoulTrapSuccess(
        const SoulTrapSuccessMessage message,
        const Victim& victim);
};

inline SoulTrapData::SoulTrapData(RE::Actor* const caster)
    : _caster(caster)
    , config(YASTMConfig::getInstance())
{}

template <typename MessageKey>
inline void SoulTrapData::_notify(const MessageKey message)
{
    if (_notifyCount < MAX_NOTIFICATION_COUNT &&
        config[BC::AllowNotifications]) {
        RE::DebugNotification(getMessage(message));
        ++_notifyCount;
    }
}

inline void SoulTrapData::_sendSoulTrapEvent(RE::Actor* const victim)
{
    if (!_isSoulTrapEventSent) {
        RE::SoulsTrapped::SendEvent(caster(), victim);
        _isSoulTrapEventSent = true;
    }
}

inline void SoulTrapData::updateLoopVariables()
{
    _victim.emplace(_victims.top());
    _victims.pop();

    if (_isInventoryMapDirty) {
        _resetInventoryData();
    }
}

inline InventoryStatus SoulTrapData::casterInventoryStatus() const
{
    // This should not happen if the class is used correctly (the class does
    // not manage these resources on its own for performance).
    assert(!_isInventoryMapDirty);
    return _casterInventoryStatus;
}

inline const SoulTrapData::InventoryItemMap& SoulTrapData::inventoryMap() const
{
    // This should not happen if the class is used correctly (the class does
    // not manage these resources on its own for performance).
    assert(!_isInventoryMapDirty);
    return _inventoryMap;
}

inline void
    SoulTrapData::notifySoulTrapFailure(const SoulTrapFailureMessage message)
{
    if (_caster->IsPlayerRef()) {
        _notify(message);
    }
}

inline void SoulTrapData::notifySoulTrapSuccess(
    const SoulTrapSuccessMessage message,
    const Victim& victim)
{
    if (_caster->IsPlayerRef() && victim.isPrimarySoul()) {
        _notify(message);
        _sendSoulTrapEvent(victim.actor());
    }
}
