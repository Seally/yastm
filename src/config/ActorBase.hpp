#pragma once

#include "Form.hpp"

namespace RE {
    class TESNPC;
} // end namespace RE

class ActorBase : public Form<RE::TESNPC> {
public:
    explicit ActorBase() {}
};
