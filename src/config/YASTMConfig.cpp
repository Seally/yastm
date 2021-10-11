#include "YASTMConfig.hpp"

#include <filesystem>

#include <toml++/toml.h>

#include <RE/A/Actor.h>
#include <RE/B/BGSDefaultObjectManager.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESGlobal.h>
#include <RE/T/TESSoulGem.h>
#include <SKSE/SKSE.h>

#include "../global.hpp"
#include "FormError.hpp"
#include "ParseError.hpp"
#include "SoulGemGroup.hpp"
#include "../formatters/TESSoulGem.hpp"
#include "../utilities/printerror.hpp"

using namespace std::literals;

YASTMConfig::YASTMConfig()
{
    // Defaults used when no associated configuration key has been set up.
    forEachConfigKey([this](const ConfigKey key, const float defaultValue) {
        _globals.emplace(key, GlobalVariable{key, defaultValue});
    });
}

void YASTMConfig::_readYASTMConfig()
{
    toml::table table;

    const std::filesystem::path configPath{"Data/YASTM.toml"sv};
    const std::string configPathStr{configPath.string()};

    try {
        table = toml::parse_file(configPathStr);

        LOG_INFO_FMT(
            "Found YASTM general configuration file: {}"sv,
            configPath.filename().string());

        const auto yastmTable = table["YASTM"];

        forEachConfigKey([&](const ConfigKey key) {
            const auto keyName = toString(key);
            const auto tomlKeyName = std::string{keyName} + "Global";

            if (const auto idArray = yastmTable[tomlKeyName].as_array();
                idArray) {
                if (_globals.contains(key)) {
                    try {
                        _globals.at(key).setFromToml(*idArray);
                    } catch (const ParseError& error) {
                        LOG_ERROR_FMT(
                            "Error while reading configuration for key \"{}\":"sv,
                            keyName);
                        printError(error, 1);
                    }
                } else {
                    LOG_ERROR_FMT(
                        "Initialized global map does not contain configuration for key \"{}\"."sv,
                        keyName);
                }
            } else {
                LOG_WARN_FMT(
                    "Form data for configuration key \"{}\" not found."sv,
                    keyName);
            }
        });
    } catch (const toml::parse_error& error) {
        LOG_WARN_FMT(
            "Error while parsing general configuration file \"{}\": {}"sv,
            configPathStr,
            error.what());
    }

#ifndef NDEBUG
    // Print the loaded configuration (we can't read the in-game forms yet.
    // Game hasn't fully initialized.)
    LOG_TRACE("Loaded configuration from TOML:"sv);

    for (const auto& [key, globalVar] : _globals) {
        if (globalVar.isConfigLoaded()) {
            LOG_TRACE_FMT("- {} = {}"sv, key, globalVar.formId());
        }
    }
#endif // NDEBUG
}

void YASTMConfig::_readIndividualConfigs()
{
    std::vector<std::filesystem::path> configPaths;

    for (const auto& entry : std::filesystem::directory_iterator("Data/"sv)) {
        if (entry.exists() && !entry.path().empty() &&
            entry.path().extension() == ".toml"sv) {
            const auto fileName = entry.path().filename();
            const auto fileNameStr = fileName.string();

            if (fileNameStr.starts_with("YASTM_"sv)) {
                LOG_INFO_FMT(
                    "Found YASTM soul gem configuration file: {}"sv,
                    fileNameStr);
                configPaths.push_back(entry.path());
            }
        }
    }

    if (configPaths.empty()) {
        throw YASTMConfigLoadError("No YASTM configuration files found.");
    }

    std::size_t validSoulGemGroupsCount = 0;

    for (const auto& configPath : configPaths) {
        toml::table table;

        std::string configPathStr = configPath.string();

        try {
            table = toml::parse_file(configPathStr);

            LOG_INFO_FMT(
                "Reading individual configuration file: {}"sv,
                configPathStr);

            validSoulGemGroupsCount += _readAndCountSoulGemGroupConfigs(table);
        } catch (const toml::parse_error& error) {
            LOG_WARN_FMT(
                "Error while parsing individual configuration file \"{}\": {}"sv,
                configPathStr,
                error.what());
        }
    }

#ifndef NDEBUG
    // Print the loaded configuration (we can't read the in-game forms yet.
    // Game hasn't fully initialized.)
    LOG_TRACE("Loaded soul gem configuration from TOML:"sv);

    for (const auto& soulGemGroup : _soulGemGroupList) {
        LOG_TRACE_FMT(
            "    {} (isReusable={}, capacity={}, priority={})"sv,
            soulGemGroup->id(),
            soulGemGroup->isReusable(),
            soulGemGroup->capacity(),
            toLoadPriorityString(soulGemGroup->rawPriority()));

        for (const auto& soulGemId : soulGemGroup->members()) {
            LOG_TRACE_FMT(
                "        [{:#08x}, {}]"sv,
                soulGemId->id(),
                soulGemId->pluginName());
        }
    }
#endif // NDEBUG

    if (validSoulGemGroupsCount <= 0) {
        throw YASTMConfigLoadError{"No valid soul gem groups found."};
    }
}

std::size_t
    YASTMConfig::_readAndCountSoulGemGroupConfigs(const toml::table& table)
{
    std::size_t validSoulGemGroupsCount = 0;

    if (const auto soulGems = table["soulGems"sv].as_array();
        soulGems != nullptr) {
        for (const toml::node& elem : *soulGems) {
            try {
                elem.visit([&, this](auto&& el) {
                    if constexpr (toml::is_table<decltype(el)>) {
                        _soulGemGroupList.emplace_back(new SoulGemGroup(el));
                        // We've found a valid soul gem group!
                        ++validSoulGemGroupsCount;
                    } else {
                        throw InvalidEntryValueTypeError{
                            "soulGems",
                            ValueType::Table,
                            "Member of 'soulGems' array must be a "
                            "table."};
                    }
                });
            } catch (const std::exception& error) {
                printError(error, 1);
            }
        }
    }

    return validSoulGemGroupsCount;
}

void YASTMConfig::checkDllDependencies(const SKSE::LoadInterface* loadInterface)
{
    forEachDLLDependencyKey([&, this](
                                const DLLDependencyKey key,
                                const char* name,
                                const char* issueIfMissing) {
        const auto pluginInfo = loadInterface->GetPluginInfo(name);
        _dependencies.emplace(key, pluginInfo);

        if (pluginInfo == nullptr) {
            // Bypass LOG_WARN(issueIfMissing) not compiling.
            LOG_WARN_FMT("{}"sv, issueIfMissing);
        }
    });
}

void YASTMConfig::readConfigs()
{
    _readYASTMConfig();
    _readIndividualConfigs();
}

void YASTMConfig::loadGameForms(RE::TESDataHandler* const dataHandler)
{
    _loadGlobalForms(dataHandler);
    _createSoulGemMap(dataHandler);
}

void YASTMConfig::_loadGlobalForms(RE::TESDataHandler* const dataHandler)
{
    using namespace std::literals;

    LOG_INFO("Loading global variable forms..."sv);

    for (auto& [key, globalVar] : _globals) {
        if (globalVar.isConfigLoaded()) {
            LOG_TRACE_FMT("Loading form for \"{}\"..."sv, key);
            try {
                globalVar.loadForm(dataHandler);
            } catch (const std::exception& error) {
                printError(error, 1);
            }
        } else {
            LOG_INFO_FMT(
                "Form ID for '{}' not specified in configuration file. Using default of {}"sv,
                globalVar.defaultValue());
        }
    }

    LOG_INFO("Listing loaded global variable forms:"sv);

    for (auto& [key, globalVar] : _globals) {
        if (globalVar.isFormLoaded()) {
            LOG_INFO_FMT("- {}: {}"sv, key, globalVar.formId());
        } else {
            LOG_INFO_FMT("- {}: Not loaded."sv, key);
        }
    }
}

void YASTMConfig::_createSoulGemMap(RE::TESDataHandler* const dataHandler)
{
    const auto addSoulGemGroupsForPriority =
        [=, this](const LoadPriority priority) {
            for (const auto& soulGemGroup : _soulGemGroupList) {
                if (soulGemGroup->priority() == priority) {
                    try {
                        _soulGemMap.addSoulGemGroup(*soulGemGroup, dataHandler);
                    } catch (const std::exception& error) {
                        printError(error);
                    }
                }
            }
        };

    addSoulGemGroupsForPriority(LoadPriority::High);
    addSoulGemGroupsForPriority(LoadPriority::Normal);
    addSoulGemGroupsForPriority(LoadPriority::Low);

    _soulGemMap.printContents();
}

YASTMConfigLoadError::YASTMConfigLoadError(const std::string& message)
    : std::runtime_error{message}
{}
