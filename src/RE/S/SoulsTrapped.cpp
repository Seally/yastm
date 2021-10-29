// From PowerOfThree's decode:
//     https://github.com/powerof3/CommonLibSSE/blob/dfa73e501c0ced55b8ce6732edc9ea4c531e9229/src/RE/S/SoulsTrapped.cpp

#include "SoulsTrapped.hpp"

#include <memory>

namespace RE {
    BSTEventSource<SoulsTrapped::Event>* SoulsTrapped::GetEventSource()
    {
        using func_t = decltype(&SoulsTrapped::GetEventSource);
        REL::Relocation<func_t> func{REL::ID(37916)};
        return func();
    }

    void SoulsTrapped::SendEvent(Actor* const a_trapper, Actor* const a_target)
    {
        Event e{a_trapper, a_target};
        auto source = GetEventSource();
        if (source) {
            source->SendEvent(std::addressof(e));
        }
    }
}
