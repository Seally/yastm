#pragma once

#include <string>
#include <string_view>

#include <RE/V/VirtualMachine.h>

#include "../global.hpp"

class PapyrusFunctionRegistry {
    using VirtualMachine = RE::BSScript::Internal::VirtualMachine;

    const std::string _className;
    VirtualMachine* const _vm;

public:
    explicit PapyrusFunctionRegistry(
        std::string_view className,
        VirtualMachine* const vm)
        : _className(className)
        , _vm(vm)
    {}

    template <typename T>
    void registerFunction(std::string_view name, T fn)
    {
        LOG_INFO_FMT("Registering function: {}.{}()", _className, name);
        _vm->RegisterFunction(name, _className, fn);
    }
};
