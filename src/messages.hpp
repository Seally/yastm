#ifndef MESSAGES_HPP
#define MESSAGES_HPP

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

inline const char* getMessage(const SoulTrapFailureMessage key)
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

inline const char* getMessage(const SoulTrapSuccessMessage key) {
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

#endif // MESSAGES_HPP
