#pragma once

#include "Form.hpp"

namespace RE {
    class Actor;
} // end namespace RE

class ActorRef : public Form<RE::Actor> {
public:
    explicit ActorRef() {}
};
