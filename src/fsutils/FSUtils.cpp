#include "FSUtils.hpp"

#include <functional>
#include <sstream>

#include <RE/V/VirtualMachine.h>

#include "global.hpp"
#include "internal/ConfigManager.hpp"
#include "../utilities/PapyrusFunctionRegistry.hpp"
#include "../utilities/printerror.hpp"

using namespace std::literals;
using RE::BSScript::Internal::VirtualMachine;

namespace {
    bool FileExists(
        VirtualMachine* vm,
        RE::VMStackID stackId,
        RE::StaticFunctionTag*,
        RE::BSFixedString path)
    {
        std::filesystem::path filePath("Data");
        filePath /= path.c_str();

        try {
            return std::filesystem::exists(filePath);
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

    bool RemoveFile(
        RE::BSScript::Internal::VirtualMachine* vm,
        RE::VMStackID stackId,
        RE::StaticFunctionTag*,
        RE::BSFixedString path)
    {
        std::filesystem::path filePath("Data");
        filePath /= path.c_str();

        try {
            return std::filesystem::remove(filePath);
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

    bool HasEntry(
        RE::BSScript::Internal::VirtualMachine* vm,
        RE::VMStackID stackId,
        RE::StaticFunctionTag*,
        ConfigManager::HandleType handle,
        RE::BSFixedString key)
    {
        if (key.length() <= 0) {
            vm->TraceStack("Key is empty", stackId);
            return false;
        }

        try {
            auto maybeConfig = ConfigManager::getInstance().getConfig(handle);

            if (maybeConfig.has_value()) {
                auto& config = maybeConfig.value().get();

                return config.has(key);
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
    bool SetValue(
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
    T GetValue(
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

    bool _registerPapyrusFunctions(VirtualMachine* const vm)
    {
        if (vm == nullptr) {
            LOG_ERROR("Couldn't get VM State"sv);
            return false;
        }

        PapyrusFunctionRegistry registry("YASTMFSUtils", vm);

        // General file system functions
        registry.registerFunction("FileExists", FileExists);
        registry.registerFunction("RemoveFile", RemoveFile);

        // Functions handling configuration files.
        registry.registerFunction("CreateConfig", CreateConfig);
        registry.registerFunction("OpenConfig", OpenConfig);
        registry.registerFunction("SaveConfig", SaveConfig);
        registry.registerFunction("CloseConfig", CloseConfig);

        registry.registerFunction("HasEntry", HasEntry);
        registry.registerFunction("GetInt", GetValue<int>);
        registry.registerFunction("GetFloat", GetValue<float>);
        registry.registerFunction("SetInt", SetValue<int>);
        registry.registerFunction("SetFloat", SetValue<float>);

        // Functions for debugging purposes.
        registry.registerFunction("GetConfigCount", GetConfigCount);
        registry.registerFunction("GetLargestHandle", GetLargestHandle);
        registry.registerFunction("GetNextHandle", GetNextHandle);
        registry.registerFunction("CloseAllConfigs", CloseAllConfigs);

        return true;
    }
} // namespace

bool registerFSUtils(const SKSE::PapyrusInterface* const papyrus)
{
    return papyrus->Register(_registerPapyrusFunctions);
}
