#pragma once

#include <RE/A/Actor.h>

#include "config/SoulSize.hpp"

class Victim {
    RE::Actor* _actor;
    SoulSize _soulSize;
    bool _isSplit;

public:
    /**
     * @brief Constructs a victim object with the soul size automatically
     * calculated from the actor's properties. This constructor is used for the
     * initial victim. 
     */
    explicit Victim(RE::Actor* actor);
    /**
     * @brief Constructs a victim object with only the soul size. This
     * constructor is used for displaced souls.
     */
    explicit Victim(SoulSize soulSize);
    /**
     * @brief Construct a victim object with both a (possibly null) actor and a
     * specified soul size. This constructor is used for split souls (the split
     * flag is set automatically).
     */
    explicit Victim(RE::Actor* actor, SoulSize soulSize);

    RE::Actor* actor() const { return _actor; }
    SoulSize soulSize() const { return _soulSize; }

    bool isPrimarySoul() const { return actor() != nullptr; }
    bool isSecondarySoul() const { return actor() == nullptr; }
    bool isSplitSoul() const { return _isSplit; }
};

inline bool operator<(const Victim& lhs, const Victim& rhs)
{
    return lhs.soulSize() < rhs.soulSize();
}

inline bool operator<=(const Victim& lhs, const Victim& rhs)
{
    return lhs.soulSize() <= rhs.soulSize();
}

inline bool operator>(const Victim& lhs, const Victim& rhs)
{
    return lhs.soulSize() > rhs.soulSize();
}

inline bool operator>=(const Victim& lhs, const Victim& rhs)
{
    return lhs.soulSize() >= rhs.soulSize();
}
