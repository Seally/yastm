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
    NoSoulGemsAvailable,
    NoSoulGemLargeEnough,
    NoSuitableSoulGem,
};

enum class MiscMessage {
    /**
     * @brief The translation string for the time taken to trap soul
     * notification. Resulting message requires processing with fmt::format()
     * with a double as an argument.
     *
     * fmt::format(getMessage(MiscMessage::TimeTakenToTrapSoul), elapsedTime)
     */
    TimeTakenToTrapSoul
};

inline constexpr const char* getMessage(const SoulTrapFailureMessage key)
{
    switch (key) {
    case SoulTrapFailureMessage::NoSoulGemsAvailable:
        return "$YASTM_Notification_NoSoulGemsAvailable";
    case SoulTrapFailureMessage::NoSuitableSoulGem:
        return "$YASTM_Notification_NoSuitableSoulGem";
    case SoulTrapFailureMessage::NoSoulGemLargeEnough:
        return "$YASTM_Notification_NoSoulGemLargeEnough";
    }

    return "";
}

inline constexpr const char* getMessage(const SoulTrapSuccessMessage key)
{
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

    return "";
}

inline constexpr const char* getMessage(const MiscMessage key)
{
    switch (key) {
    case MiscMessage::TimeTakenToTrapSoul:
        if (YASTMConfig::getInstance().isDllLoaded(
                DllDependencyKey::ScaleformTranslationPlusPlus)) {
            return "$YASTM_Notification_TimeTakenToTrapSoul{{{:.7f}}}";
        }

        return "Time taken to trap soul: {:.7f} seconds";
    }

    return "";
}
