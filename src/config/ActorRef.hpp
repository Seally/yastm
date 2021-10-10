#pragma once

#ifdef YASTM_SOULDIVERSION_ENABLED

#include "Form.hpp"

namespace RE {
    class Actor;
} // end namespace RE

class ActorRef : public Form<RE::Actor> {
public:
    explicit ActorRef() {}
};

#endif // YASTM_SOULDIVERSION_ENABLED
