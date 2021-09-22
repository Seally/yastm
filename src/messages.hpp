#ifndef MESSAGES_HPP
#define MESSAGES_HPP

enum class Message {
    NoSoulGemLargeEnough,
    SoulCaptured,
    SoulDisplaced,
    SoulShrunk
};

inline const char* getMessage(const Message key)
{
    switch (key) {
    case Message::NoSoulGemLargeEnough:
        return "$YASTM_Notification_NoSoulGemLargeEnough";
    case Message::SoulCaptured:
        return "$YASTM_Notification_SoulCaptured";
    case Message::SoulDisplaced:
        return "$YASTM_Notification_SoulDisplaced";
    case Message::SoulShrunk:
        return "$YASTM_Notification_SoulShrunk";
    }

    return "";
}

#endif // MESSAGES_HPP
