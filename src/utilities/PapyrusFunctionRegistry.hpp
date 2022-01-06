#pragma once

#include <string>
#include <string_view>

#include <RE/V/VirtualMachine.h>

#include "../global.hpp"

class PapyrusFunctionRegistry {
    using VirtualMachine = RE::BSScript::Internal::VirtualMachine;

    const std::string className_;
    VirtualMachine* const vm_;

public:
    explicit PapyrusFunctionRegistry(
        std::string_view className,
        VirtualMachine* const vm)
        : className_(className)
        , vm_(vm)
    {}

    template <typename T>
    void registerFunction(std::string_view name, T fn)
    {
        LOG_INFO_FMT("Registering function: {}.{}()", className_, name);
        vm_->RegisterFunction(name, className_, fn);
    }
};
