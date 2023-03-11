#pragma once

#include "config/DllDependencyKey.hpp"
#include "config/YASTMConfig.hpp"

enum class SoulTrapSuccessMessage {
    SoulCaptured,
    SoulDisplaced,
    SoulShrunk,
    SoulSplit,
};

enum class SoulTrapFailureMessage {
    /**
     * @brief Caster doesn't own any soul gems.
    */
    NoSoulGemsOwned,
    /**
     * @brief All of the caster's soul gems have been max-filled.
     */
    AllSoulGemsFilled,
    /**
     * @brief Caster has soul gems, but soul doesn't fit into any of them
     * (splitting/shrinking disabled).
     */
    NoSoulGemLargeEnough,
    /**
     * @brief Caster has soul gems, but none of them are suitable for
     * capturing the soul (splitting/shrinking enabled). This is a catch-all
     * when no other condition is satisfied.
     */
    NoSuitableSoulGem,
    /**
     * @brief The soul was lost due to the caster's improficiency (too low
     * level).
     */
    SoulLost
};

enum class MiscMessage {
    /**
     * @brief The translation string for the time taken to trap soul
     * notification. Resulting message requires processing with fmt::format()
     * with a double as an argument.
     *
     * @example
     * fmt::format(getMessage(MiscMessage::TimeTakenToTrapSoul), elapsedTime)
     */
    TimeTakenToTrapSoul,
    CannotFindSoulGemBaseForm,
};

inline constexpr const char*
    getMessage(const SoulTrapFailureMessage key) noexcept
{
    switch (key) {
    case SoulTrapFailureMessage::NoSoulGemsOwned:
        return "$YASTM_Notification_NoSoulGemsOwned";
    case SoulTrapFailureMessage::AllSoulGemsFilled:
        return "$YASTM_Notification_AllSoulGemsFilled";
    case SoulTrapFailureMessage::NoSoulGemLargeEnough:
        return "$YASTM_Notification_NoSoulGemLargeEnough";
    case SoulTrapFailureMessage::NoSuitableSoulGem:
        return "$YASTM_Notification_NoSuitableSoulGem";
    case SoulTrapFailureMessage::SoulLost:
        return "$YASTM_Notification_SoulLost";
    }

    return "";
}

inline constexpr const char*
    getMessage(const SoulTrapSuccessMessage key, const bool degraded) noexcept
{
    if (degraded) {
        switch (key) {
        case SoulTrapSuccessMessage::SoulCaptured:
            return "$YASTM_Notification_SoulCaptured_Degraded";
        case SoulTrapSuccessMessage::SoulDisplaced:
            return "$YASTM_Notification_SoulDisplaced_Degraded";
        case SoulTrapSuccessMessage::SoulShrunk:
            return "$YASTM_Notification_SoulShrunk_Degraded";
        case SoulTrapSuccessMessage::SoulSplit:
            return "$YASTM_Notification_SoulSplit_Degraded";
        }
    } else {
        switch (key) {
        case SoulTrapSuccessMessage::SoulCaptured:
            return "$YASTM_Notification_SoulCaptured";
        case SoulTrapSuccessMessage::SoulDisplaced:
            return "$YASTM_Notification_SoulDisplaced";
        case SoulTrapSuccessMessage::SoulShrunk:
            return "$YASTM_Notification_SoulShrunk";
        case SoulTrapSuccessMessage::SoulSplit:
            return "$YASTM_Notification_SoulSplit";
        }
    }

    return "";
}

inline const char* getMessage(const MiscMessage key)
{
    switch (key) {
    case MiscMessage::TimeTakenToTrapSoul:
        if (YASTMConfig::getInstance().isDllLoaded(
                DLLDependencyKey::ScaleformTranslationPlusPlus)) {
            return "$YASTM_Notification_TimeTakenToTrapSoul{{{:.7f}}}";
        }

        return "Time taken to trap soul: {:.7f} seconds";
    case MiscMessage::CannotFindSoulGemBaseForm:
        // We don't want a translation string for an error message that should
        // never happen.
        return "ERROR: Soul gem not consumed because no base form was found.";
    }

    return "";
}
