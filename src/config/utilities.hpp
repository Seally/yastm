#pragma once

#include <optional>

#include <RE/T/TESSoulGem.h>

#include "SoulGemMap.hpp"
#include "YASTMConfig.hpp"

/**
 * @brief Returns the soul gem map.
 *
 * @param[in] config The configuration instance. Defaults to calling
 * YASTMConfig::getInstance() if not provided.
 */
[[nodiscard]] inline const SoulGemMap&
    getSoulGemMap(const YASTMConfig& config = YASTMConfig::getInstance())
{
    return config.soulGemMap();
}

/**
 * @brief Gets the base form (empty version) of the given soul gem. This
 * function first consults the linked soul gem field (NAM0) of the given soul
 * gem first, then falls back to looking up the soul gem map.
 *
 * @param[in] soulGem The soul gem to look up the base form for.
 *
 * @returns The pointer to the base soul gem form, or nullptr if lookup fails.
 */
[[nodiscard]] inline RE::TESSoulGem*
    getSoulGemBaseForm(RE::TESSoulGem* const soulGem)
{
    // Try looking up the linked soul gem first to get the base form (empty
    // version of this soul gem).
    RE::TESSoulGem* baseSoulGem = soulGem->linkedSoulGem;

    if (baseSoulGem == nullptr) {
        // If that fails, look up the base form provided by the soul gem
        // map.
        baseSoulGem = getSoulGemMap().getBaseFormOf(soulGem);
    }

    return baseSoulGem;
}

/**
 * @brief Gets the base form (empty version) of the given soul gem. This
 * function first consults the linked soul gem field (NAM0) of the given soul
 * gem first, then falls back to looking up the soul gem map.
 *
 * @param[in] soulGem The soul gem to look up the base form for.
 * @param[in] soulGemMap A SoulGemMap instance.
 *
 * @returns The pointer to the base soul gem form, or nullptr if lookup fails.
 */
[[nodiscard]] inline RE::TESSoulGem* getSoulGemBaseForm(
    RE::TESSoulGem* const soulGem,
    const SoulGemMap& soulGemMap)
{
    // Try looking up the linked soul gem first to get the base form (empty
    // version of this soul gem).
    RE::TESSoulGem* baseSoulGem = soulGem->linkedSoulGem;

    if (baseSoulGem == nullptr) {
        // If that fails, look up the base form provided by the soul gem
        // map.
        baseSoulGem = soulGemMap.getBaseFormOf(soulGem);
    }

    return baseSoulGem;
}
