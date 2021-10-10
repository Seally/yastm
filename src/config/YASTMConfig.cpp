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
        throw std::runtime_error("No YASTM configuration files found.");
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
#ifdef YASTM_SOULDIVERSION_ENABLED
            _readDiversionIgnoreConfigs(table);
#endif // YASTM_SOULDIVERSION_ENABLED
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

#ifdef YASTM_SOULDIVERSION_ENABLED
template <typename T>
void _readDiversionIgnoreConfig(
    const DiversionConfigKey key,
    const toml::table& diversionTable,
    std::vector<T>& outputList)
{
    const auto keyString = toString(key);

    try {
        if (const auto ignoreConfigArray = diversionTable[keyString].as_array();
            ignoreConfigArray != nullptr) {
            std::size_t index = 0;

            for (const toml::node& elem : *ignoreConfigArray) {
                try {
                    elem.visit([&](auto&& el) {
                        if constexpr (toml::is_array<decltype(el)>) {
                            auto& output = outputList.emplace_back();

                            output.setFromToml(el);
                        } else {
                            throw EntryError(
                                index,
                                fmt::format(
                                    FMT_STRING("{}[{}] is not an array"),
                                    keyString,
                                    index));
                        }
                    });
                } catch (...) {
                    std::throw_with_nested(EntryError(
                        index,
                        fmt::format(
                            FMT_STRING("Invalid form ID entry at {}[{}]"sv),
                            keyString,
                            index)));
                }
                ++index;
            }
        }
    } catch (const std::exception& error) {
        LOG_ERROR_FMT("Error while reading the entry for \"{}\":"sv, keyString);
        printError(error, 1);
    }
}

void YASTMConfig::_readDiversionIgnoreConfigs(const toml::table& table)
{
    const auto yastmTable = table["YASTM"];

    if (const auto diversionTable = yastmTable["diversion"].as_table();
        diversionTable != nullptr) {
        _readDiversionIgnoreConfig(
            DiversionConfigKey::ActorBaseIgnoreList,
            *diversionTable,
            _actorBaseList);
        _readDiversionIgnoreConfig(
            DiversionConfigKey::ActorRefIgnoreList,
            *diversionTable,
            _actorRefList);
    }

#    ifndef NDEBUG
    // Print the loaded configuration (we can't read the in-game forms yet.
    // Game hasn't fully initialized.)
    LOG_TRACE("Loaded soul diversion ignore list from TOML:");

    LOG_TRACE("Base actors:"sv);

    for (const auto& actorBase : _actorBaseList) {
        if (actorBase.isConfigLoaded()) {
            LOG_TRACE_FMT("- {}"sv, actorBase.formId());
        }
    }

    LOG_TRACE("Actor references:"sv);

    for (const auto& actorRef : _actorRefList) {
        if (actorRef.isConfigLoaded()) {
            LOG_TRACE_FMT("- {}"sv, actorRef.formId());
        }
    }
#    endif // NDEBUG
}
#endif // YASTM_SOULDIVERSION_ENABLED

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
#ifdef YASTM_SOULDIVERSION_ENABLED
    _loadDiversionActorIgnoreList(dataHandler);
#endif // YASTM_SOULDIVERSION_ENABLED
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

#ifdef YASTM_SOULDIVERSION_ENABLED
void YASTMConfig::_loadDiversionActorIgnoreList(
    RE::TESDataHandler* const dataHandler)
{
    for (auto& actorBase : _actorBaseList) {
        try {
            actorBase.loadForm(dataHandler);

            if (actorBase.isFormLoaded()) {
                const auto form = actorBase.form();

                _diversionActorIgnoreList.emplace(form->GetFormID());

                LOG_INFO(
                    "Actor base \"{}\" {} added to Soul Diversion ignore "
                    "list.",
                    form->GetName(),
                    actorBase.formId());
            }
        } catch (const std::exception& error) {
            printError(error);
        }
    }

    for (auto& actorRef : _actorRefList) {
        try {
            actorRef.loadForm(dataHandler);

            if (actorRef.isFormLoaded()) {
                const auto form = actorRef.form();

                _diversionActorIgnoreList.emplace(form->GetFormID());

                LOG_INFO(
                    "Actor reference \"{}\" {} added to Soul Diversion ignore "
                    "list.",
                    form->GetName(),
                    actorRef.formId());
            }
        } catch (const std::exception& error) {
            printError(error);
        }
    }
}
#endif // YASTM_SOULDIVERSION_ENABLED

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
