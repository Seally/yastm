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
#include "../utilities/containerutils.hpp"
#include "../utilities/printerror.hpp"

using namespace std::literals;

namespace {
    template <typename KeyType>
    void readGlobalVariableConfigs_(
        const KeyType key,
        const toml::node_view<toml::node>& table,
        YASTMConfig::GlobalVarMap<KeyType>& map)
    {
        const auto keyName = toString(key);
        const auto tomlKeyName = std::string(keyName) + "Global";

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

    template <typename KeyType>
    void loadGlobalFormsIn_(
        YASTMConfig::GlobalVarMap<KeyType>& map,
        RE::TESDataHandler* const dataHandler)
    {
        for (auto& [key, globalVar] : map) {
            if (globalVar.isConfigLoaded()) {
                LOG_INFO_FMT("Loading form for \"{}\"..."sv, key);
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
    void printLoadedGlobalForms_(const YASTMConfig::GlobalVarMap<KeyType>& map)
    {
        for (auto& [key, globalVar] : map) {
            if (globalVar.isFormLoaded()) {
                LOG_INFO_FMT("- {}: {}"sv, key, globalVar.formId());
            } else {
                LOG_INFO_FMT("- {}: Not loaded."sv, key);
            }
        }
    }
} // namespace

YASTMConfig::YASTMConfig()
{
    // Defaults used when no associated configuration key has been set up.
    forEachBoolConfigKey(
        [this](const BoolConfigKey key, const float defaultValue) {
            globalBools_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(key),
                std::forward_as_tuple(key, defaultValue));
        });

    forEachEnumConfigKey(
        [this](const EnumConfigKey key, const float defaultValue) {
            globalEnums_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(key),
                std::forward_as_tuple(key, defaultValue));
        });
}

void YASTMConfig::loadYASTMConfigFile_()
{
    toml::table table;

    const std::filesystem::path configPath("Data/YASTM.toml"sv);
    const std::string configPathStr(configPath.string());

    try {
        table = toml::parse_file(configPathStr);

        LOG_INFO_FMT(
            "Found YASTM general configuration file: {}"sv,
            configPath.filename().string());

        const auto yastmTable = table["YASTM"];

        forEachBoolConfigKey([&, this](const BoolConfigKey key) {
            readGlobalVariableConfigs_(key, yastmTable, globalBools_);
        });

        forEachEnumConfigKey([&, this](const EnumConfigKey key) {
            readGlobalVariableConfigs_(key, yastmTable, globalEnums_);
        });
    } catch (const toml::parse_error& error) {
        LOG_WARN_FMT(
            "Error while parsing general configuration file \"{}\": {}"sv,
            configPathStr,
            error.what());
    }

    // Print the loaded configuration (we can't read the in-game forms yet.
    // Game hasn't fully initialized.)
    LOG_INFO("Loaded configuration from TOML:"sv);

    for (const auto& [key, globalVar] : globalBools_) {
        if (globalVar.isConfigLoaded()) {
            LOG_INFO_FMT("- {} = {}"sv, key, globalVar.formId());
        }
    }

    for (const auto& [key, globalVar] : globalEnums_) {
        if (globalVar.isConfigLoaded()) {
            LOG_INFO_FMT("- {} = {}"sv, key, globalVar.formId());
        }
    }
}

void YASTMConfig::loadIndividualConfigFiles_()
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

            validSoulGemGroupsCount += readAndCountSoulGemGroupConfigs_(table);
        } catch (const toml::parse_error& error) {
            LOG_WARN_FMT(
                "Error while parsing individual configuration file \"{}\": {}"sv,
                configPathStr,
                error.what());
        }
    }

    // Print the loaded configuration (we can't read the in-game forms yet.
    // Game hasn't fully initialized.)
    LOG_INFO("Loaded soul gem configuration from TOML:"sv);

    for (const auto& soulGemGroup : soulGemGroupList_) {
        LOG_INFO_FMT(
            "    {} (isReusable={}, capacity={}, priority={})"sv,
            soulGemGroup.id(),
            soulGemGroup.isReusable(),
            soulGemGroup.capacity(),
            toString(soulGemGroup.rawPriority()));

        for (const auto& soulGemId : soulGemGroup.members()) {
            LOG_INFO_FMT("        {}"sv, soulGemId);
        }
    }

    if (validSoulGemGroupsCount <= 0) {
        throw YASTMConfigLoadError("No valid soul gem groups found.");
    }
}

std::size_t
    YASTMConfig::readAndCountSoulGemGroupConfigs_(const toml::table& table)
{
    std::size_t validSoulGemGroupsCount = 0;

    if (const auto soulGems = table["soulGems"sv].as_array();
        soulGems != nullptr) {
        for (const toml::node& elem : *soulGems) {
            try {
                elem.visit([&, this](auto&& el) {
                    if constexpr (toml::is_table<decltype(el)>) {
                        soulGemGroupList_.emplace_back(el);
                        // We've found a valid soul gem group!
                        ++validSoulGemGroupsCount;
                    } else {
                        throw ParseError(
                            "Member of 'soulGems' array must be a table.");
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
        dependencies_.emplace(key, pluginInfo);

        if (pluginInfo == nullptr) {
            // Bypass LOG_WARN(issueIfMissing) not compiling.
            LOG_WARN_FMT("{}"sv, issueIfMissing);
        }
    });
}

void YASTMConfig::loadConfigFiles_()
{
    LOG_INFO("Loading configuration files...");
    loadYASTMConfigFile_();
    loadIndividualConfigFiles_();
}

void YASTMConfig::loadGameForms_(RE::TESDataHandler* const dataHandler)
{
    LOG_INFO("Loading game forms...");
    loadGlobalForms_(dataHandler);
    createSoulGemMap_(dataHandler);
}

void YASTMConfig::loadConfig(RE::TESDataHandler* const dataHandler)
{
    static bool isFirstRun = true;
    std::lock_guard lock(mutex_);

    if (!isFirstRun) {
        clear();
    }

    isFirstRun = false;
    loadConfigFiles_();
    loadGameForms_(dataHandler);
}

void YASTMConfig::clear()
{
    LOG_INFO("Clearing configuration data...");

    // Clear the loaded data (form ID and game form) but leave the default
    // values intact.
    for (auto& [key, globalBool] : globalBools_) { globalBool.clear(); }
    for (auto& [key, globalEnum] : globalEnums_) { globalEnum.clear(); }

    clearContainer(soulGemGroupList_);
    soulGemMap_.clear();
    // This doesn't need to be cleared because the list won't change until the
    // game fully restarts.
    //dependencies_ =
    //    std::unordered_map<DLLDependencyKey, const SKSE::PluginInfo*>();
}

void YASTMConfig::loadGlobalForms_(RE::TESDataHandler* const dataHandler)
{
    using namespace std::literals;

    LOG_INFO("Loading global variable forms..."sv);
    loadGlobalFormsIn_(globalBools_, dataHandler);
    loadGlobalFormsIn_(globalEnums_, dataHandler);

    LOG_INFO("Listing loaded global variable forms:"sv);
    printLoadedGlobalForms_(globalBools_);
    printLoadedGlobalForms_(globalEnums_);
}

void YASTMConfig::createSoulGemMap_(RE::TESDataHandler* const dataHandler)
{
    soulGemMap_.initializeWith(dataHandler, [this](SoulGemMap::Transaction& t) {
        for (const auto& group : soulGemGroupList_) {
            t.addSoulGemGroup(group);
        }
    });

    soulGemMap_.printContents();
}
