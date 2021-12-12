#pragma once

#include <RE/A/Actor.h>
#include <RE/P/PlayerCharacter.h>

#include "../global.hpp"
#include "../config/ConfigKey.hpp"
#include "../config/YASTMConfig.hpp"

bool trapSoul(RE::Actor* caster, RE::Actor* victim);

inline RE::Actor* getProxyCaster(RE::Actor* caster)
{
    const auto& config = YASTMConfig::getInstance();

    if (config.getGlobalBool(BoolConfigKey::AllowSoulDiversion) &&
        caster->IsPlayerTeammate()) {
        const auto playerActor = RE::PlayerCharacter::GetSingleton();

        if (playerActor != nullptr) {
            LOG_TRACE("Soul trap diverted to player."sv);
            return playerActor;
        } else {
            LOG_WARN("Failed to find player reference for soul diversion.");
        }
    }

    return caster;
}
