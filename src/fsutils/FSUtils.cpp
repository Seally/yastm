#include "FSUtils.hpp"

#include <functional>
#include <sstream>

#include <RE/V/VirtualMachine.h>

#include "global.hpp"
#include "internal/ConfigManager.hpp"
#include "../utilities/printerror.hpp"

using namespace std::literals;
using RE::BSScript::Internal::VirtualMachine;

ConfigManager::HandleType OpenConfig(
    RE::BSScript::Internal::VirtualMachine* vm,
    RE::VMStackID stackId,
    RE::StaticFunctionTag*,
    RE::BSFixedString path)
{
    if (path.length() <= 0) {
        vm->TraceStack("File path is empty", stackId);
        return 0;
    }

    std::filesystem::path filePath("Data");
    filePath /= path.c_str();

    try {
        return ConfigManager::getInstance().openConfig(filePath);
    } catch (const std::exception& error) {
        std::stringstream stream;

        printErrorToStream(error, stream);
        vm->TraceStack(
            stream.str().c_str(),
            stackId,
            RE::BSScript::ErrorLogger::Severity::kInfo);
    }

    return 0;
}

ConfigManager::HandleType CreateConfig(
    RE::BSScript::Internal::VirtualMachine* vm,
    RE::VMStackID stackId,
    RE::StaticFunctionTag*)
{
    try {
        return ConfigManager::getInstance().createConfig();
    } catch (const std::exception& error) {
        std::stringstream stream;

        printErrorToStream(error, stream);
        vm->TraceStack(
            stream.str().c_str(),
            stackId,
            RE::BSScript::ErrorLogger::Severity::kInfo);
    }

    return 0;
}

bool SaveConfig(
    RE::BSScript::Internal::VirtualMachine* vm,
    RE::VMStackID stackId,
    RE::StaticFunctionTag*,
    ConfigManager::HandleType handle,
    RE::BSFixedString path)
{
    std::filesystem::path filePath("Data");
    filePath /= path.c_str();

    try {
        ConfigManager::getInstance().saveConfig(handle, filePath);

        return true;
    } catch (const std::exception& error) {
        std::stringstream stream;

        printErrorToStream(error, stream);
        vm->TraceStack(
            stream.str().c_str(),
            stackId,
            RE::BSScript::ErrorLogger::Severity::kInfo);
    }

    return false;
}

void CloseConfig(
    RE::BSScript::Internal::VirtualMachine* vm,
    RE::VMStackID stackId,
    RE::StaticFunctionTag*,
    ConfigManager::HandleType handle)
{
    try {
        ConfigManager::getInstance().closeConfig(handle);
    } catch (const std::exception& error) {
        std::stringstream stream;

        printErrorToStream(error, stream);
        vm->TraceStack(
            stream.str().c_str(),
            stackId,
            RE::BSScript::ErrorLogger::Severity::kInfo);
    }
}

template <typename T>
bool SaveValue(
    RE::BSScript::Internal::VirtualMachine* vm,
    RE::VMStackID stackId,
    RE::StaticFunctionTag*,
    ConfigManager::HandleType handle,
    RE::BSFixedString key,
    T value)
{
    if (key.length() <= 0) {
        vm->TraceStack("Key is empty", stackId);
        return false;
    }

    try {
        auto maybeConfig = ConfigManager::getInstance().getConfig(handle);

        if (maybeConfig.has_value()) {
            auto& config = maybeConfig.value().get();
            config.set(key, value);

            return true;
        }
    } catch (const std::exception& error) {
        std::stringstream stream;

        printErrorToStream(error, stream);
        vm->TraceStack(
            stream.str().c_str(),
            stackId,
            RE::BSScript::ErrorLogger::Severity::kInfo);
    }

    return false;
}

template <typename T>
T LoadValue(
    RE::BSScript::Internal::VirtualMachine* vm,
    RE::VMStackID stackId,
    RE::StaticFunctionTag*,
    ConfigManager::HandleType handle,
    RE::BSFixedString key,
    T defaultValue)
{
    if (key.length() <= 0) {
        vm->TraceStack("Key is empty", stackId);
        return defaultValue;
    }

    try {
        auto maybeConfig = ConfigManager::getInstance().getConfig(handle);

        if (maybeConfig.has_value()) {
            auto& config = maybeConfig.value().get();
            return config.get(key, defaultValue);
        }
    } catch (const std::exception& error) {
        std::stringstream stream;

        printErrorToStream(error, stream);
        vm->TraceStack(
            stream.str().c_str(),
            stackId,
            RE::BSScript::ErrorLogger::Severity::kInfo);
    }

    return defaultValue;
}

int GetConfigCount(
    RE::BSScript::Internal::VirtualMachine* vm,
    RE::VMStackID stackId,
    RE::StaticFunctionTag*)
{
    try {
        // This truncates the value since std::size_t is defined (in this
        // writing's context) as 'unsigned long long' while we're return an int.
        //
        // However, this is unavoidable since Papyrus has no support for an
        // integer of this size and, short of an actual bug causing a bunch of
        // open configurations, should never happen anyway.
        return static_cast<int>(ConfigManager::getInstance().size());
    } catch (const std::exception& error) {
        std::stringstream stream;

        printErrorToStream(error, stream);
        vm->TraceStack(
            stream.str().c_str(),
            stackId,
            RE::BSScript::ErrorLogger::Severity::kInfo);
    }

    return 0;
}

ConfigManager::HandleType GetLargestHandle(
    RE::BSScript::Internal::VirtualMachine* vm,
    RE::VMStackID stackId,
    RE::StaticFunctionTag*)
{
    try {
        return ConfigManager::getInstance().getLargestHandle();
    } catch (const std::exception& error) {
        std::stringstream stream;

        printErrorToStream(error, stream);
        vm->TraceStack(
            stream.str().c_str(),
            stackId,
            RE::BSScript::ErrorLogger::Severity::kInfo);
    }

    return 0;
}

ConfigManager::HandleType GetNextHandle(
    RE::BSScript::Internal::VirtualMachine* vm,
    RE::VMStackID stackId,
    RE::StaticFunctionTag*)
{
    try {
        return ConfigManager::getInstance().getNextHandle();
    } catch (const std::exception& error) {
        std::stringstream stream;

        printErrorToStream(error, stream);
        vm->TraceStack(
            stream.str().c_str(),
            stackId,
            RE::BSScript::ErrorLogger::Severity::kInfo);
    }

    return 0;
}

void CloseAllConfigs(
    RE::BSScript::Internal::VirtualMachine* vm,
    RE::VMStackID stackId,
    RE::StaticFunctionTag*)
{
    try {
        ConfigManager::getInstance().closeAllConfigs();
    } catch (const std::exception& error) {
        std::stringstream stream;

        printErrorToStream(error, stream);
        vm->TraceStack(
            stream.str().c_str(),
            stackId,
            RE::BSScript::ErrorLogger::Severity::kInfo);
    }
}

class _FunctionRegistry {
    const std::string _className;
    VirtualMachine* const _vm;

public:
    explicit _FunctionRegistry(
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

bool _registerPapyrusFunctions(VirtualMachine* const vm)
{
    if (vm == nullptr) {
        LOG_ERROR("Couldn't get VM State"sv);
        return false;
    }

    _FunctionRegistry registry("YASTMFSUtils", vm);

    registry.registerFunction("CreateConfig", CreateConfig);
    registry.registerFunction("OpenConfig", OpenConfig);
    registry.registerFunction("SaveConfig", SaveConfig);
    registry.registerFunction("CloseConfig", CloseConfig);

    registry.registerFunction("SaveInt", SaveValue<int>);
    registry.registerFunction("SaveFloat", SaveValue<float>);
    registry.registerFunction("LoadInt", LoadValue<int>);
    registry.registerFunction("LoadFloat", LoadValue<float>);

    registry.registerFunction("GetConfigCount", GetConfigCount);
    registry.registerFunction("GetLargestHandle", GetLargestHandle);
    registry.registerFunction("GetNextHandle", GetNextHandle);
    registry.registerFunction("CloseAllConfigs", CloseAllConfigs);

    return true;
}

bool registerFSUtils(const SKSE::PapyrusInterface* const papyrus)
{
    return papyrus->Register(_registerPapyrusFunctions);
}
