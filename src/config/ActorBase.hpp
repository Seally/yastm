#pragma once

#ifdef YASTM_SOULDIVERSION_ENABLED

#include "Form.hpp"

namespace RE {
    class TESNPC;
} // end namespace RE

class ActorBase : public Form<RE::TESNPC> {
public:
    explicit ActorBase() {}
};

#endif // YASTM_SOULDIVERSION_ENABLED
