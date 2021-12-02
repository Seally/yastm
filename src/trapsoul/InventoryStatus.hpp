#pragma once

enum class InventoryStatus {
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
