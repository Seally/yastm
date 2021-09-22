#ifndef VICTIM_HPP
#define VICTIM_HPP

#include <RE/A/Actor.h>

#include "config/SoulSize.hpp"

class Victim {
    RE::Actor* _actor;
    SoulSize _soulSize;

public:
    explicit Victim(RE::Actor* actor);
    explicit Victim(SoulSize soulSize);

    RE::Actor* actor() const { return _actor; }
    SoulSize soulSize() const { return _soulSize; }

    bool isDisplacedSoul() const { return actor() == nullptr; }
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

#endif // VICTIM_HPP
