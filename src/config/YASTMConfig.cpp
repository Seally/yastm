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
#include "../utilities/printerror.hpp"

using namespace std::literals;

YASTMConfig::YASTMConfig()
{
    // Defaults used when no associated configuration key has been set up.
    forEachBoolConfigKey(
        [this](const BoolConfigKey key, const float defaultValue) {
            _globalBools.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(key),
                std::forward_as_tuple(key, defaultValue));
        });

    forEachEnumConfigKey(
        [this](const EnumConfigKey key, const float defaultValue) {
            _globalEnums.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(key),
                std::forward_as_tuple(key, defaultValue));
        });
}

template <typename KeyType>
void _readGlobalVariableConfigs(
    const KeyType key,
    const toml::node_view<toml::node>& table,
    YASTMConfig::GlobalVariableMap<KeyType>& map)
{
    const auto keyName = toString(key);
    const auto tomlKeyName = std::string{keyName} + "Global";

    if (const auto idArray = table[tomlKeyName].as_array(); idArray) {
        if (map.contains(key)) {
            try {
                map.at(key).setFromToml(*idArray);
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

        forEachBoolConfigKey([&, this](const BoolConfigKey key) {
            _readGlobalVariableConfigs(key, yastmTable, _globalBools);
        });

        forEachEnumConfigKey([&, this](const EnumConfigKey key) {
            _readGlobalVariableConfigs(key, yastmTable, _globalEnums);
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

    for (const auto& [key, globalVar] : _globalBools) {
        if (globalVar.isConfigLoaded()) {
            LOG_TRACE_FMT("- {} = {}"sv, key, globalVar.formId());
        }
    }

    for (const auto& [key, globalVar] : _globalEnums) {
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
            soulGemGroup.id(),
            soulGemGroup.isReusable(),
            soulGemGroup.capacity(),
            toString(soulGemGroup.rawPriority()));

        for (const auto& soulGemId : soulGemGroup.members()) {
            LOG_TRACE_FMT("        {}"sv, soulGemId);
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
                        _soulGemGroupList.emplace_back(el);
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

template <typename KeyType>
void _loadGlobalFormsIn(
    YASTMConfig::GlobalVariableMap<KeyType>& map,
    RE::TESDataHandler* const dataHandler)
{
    for (auto& [key, globalVar] : map) {
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
                key,
                globalVar.defaultValue());
        }
    }
}

template <typename KeyType>
void _printLoadedGlobalForms(const YASTMConfig::GlobalVariableMap<KeyType>& map)
{
    for (auto& [key, globalVar] : map) {
        if (globalVar.isFormLoaded()) {
            LOG_INFO_FMT("- {}: {}"sv, key, globalVar.formId());
        } else {
            LOG_INFO_FMT("- {}: Not loaded."sv, key);
        }
    }
}

void YASTMConfig::_loadGlobalForms(RE::TESDataHandler* const dataHandler)
{
    using namespace std::literals;

    LOG_INFO("Loading global variable forms..."sv);
    _loadGlobalFormsIn(_globalBools, dataHandler);
    _loadGlobalFormsIn(_globalEnums, dataHandler);

    LOG_INFO("Listing loaded global variable forms:"sv);
    _printLoadedGlobalForms(_globalBools);
    _printLoadedGlobalForms(_globalEnums);
}

void YASTMConfig::_createSoulGemMap(RE::TESDataHandler* const dataHandler)
{
    _soulGemMap.initializeWith(dataHandler, [this](SoulGemMap::Transaction& t) {
        for (const auto& group : _soulGemGroupList) {
            t.addSoulGemGroup(group);
        }
    });

    _soulGemMap.printContents();
}

YASTMConfigLoadError::YASTMConfigLoadError(const std::string& message)
    : std::runtime_error{message}
{}
