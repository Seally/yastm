#pragma once

#include <mutex>

#include <SKSE/API.h>

#include "global.hpp"

inline void allocateTrampoline()
{
    static std::once_flag flag;

    std::call_once(flag, []() {
        constexpr std::size_t TRAMPOLINE_SIZE_BYTES = 100;
        LOG_INFO_FMT(
            "Allocating trampoline with size: {}B",
            TRAMPOLINE_SIZE_BYTES);
        SKSE::AllocTrampoline(TRAMPOLINE_SIZE_BYTES);
    });
}
