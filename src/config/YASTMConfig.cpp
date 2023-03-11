#include "YASTMConfig.hpp"

#include <algorithm>
#include <filesystem>
#include <utility>

#include <toml++/toml.h>

#include <RE/B/BGSDefaultObjectManager.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESGlobal.h>
#include <RE/T/TESSoulGem.h>
#include <SKSE/SKSE.h>

#include "../global.hpp"
#include "FormError.hpp"
#include "ParseError.hpp"
#include "SoulGemGroup.hpp"
#include "../formatters/TESForm.hpp"
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

        const auto& locatorNode = table[tomlKeyName];

        if (const auto formIdArray = locatorNode.as_array();
            formIdArray != nullptr) {
            if (map.contains(key)) {
                try {
                    map.at(key).setFromTomlArray(*formIdArray);
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
        } else if (const auto edidString = locatorNode.as_string();
                   edidString != nullptr) {
            if (map.contains(key)) {
                try {
                    map.at(key).setFromTomlString(edidString->get());
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
    void printGlobalForms_(const YASTMConfig::GlobalVarMap<KeyType>& map)
    {
        for (const auto& [key, globalVar] : map) {
            if (globalVar.isConfigLoaded()) {
                std::visit(
                    [key](auto&& formLocator) {
                        LOG_INFO_FMT("- {} = {}"sv, key, formLocator);
                    },
                    globalVar.formLocator());
            }
        }
    }

    template <typename KeyType>
    void printLoadedGlobalForms_(const YASTMConfig::GlobalVarMap<KeyType>& map)
    {
        for (const auto& [key, globalVar] : map) {
            if (globalVar.isFormLoaded()) {
                LOG_INFO_FMT(
                    "- {} = {}",
                    key,
                    *static_cast<RE::TESForm*>(globalVar.form()));
            } else {
                LOG_INFO_FMT("- {}: Not loaded."sv, key);
            }
        }
    }

    const std::array SOULTRAP_THRESHOLD_SOULSIZE_KEYS_ = {
        IntConfigKey::SoulTrapThresholdPetty,
        IntConfigKey::SoulTrapThresholdLesser,
        IntConfigKey::SoulTrapThresholdCommon,
        IntConfigKey::SoulTrapThresholdGreater,
        IntConfigKey::SoulTrapThresholdGrand,
        IntConfigKey::SoulTrapThresholdBlack,
    };
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

    forEachIntConfigKey(
        [this](const IntConfigKey key, const float defaultValue) {
            globalInts_.emplace(
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
            "Found YASTM general configuration file: {}",
            configPath.filename().string());

        const auto yastmTable = table["YASTM"];

        forEachBoolConfigKey([&, this](const BoolConfigKey key) {
            readGlobalVariableConfigs_(key, yastmTable, globalBools_);
        });

        forEachEnumConfigKey([&, this](const EnumConfigKey key) {
            readGlobalVariableConfigs_(key, yastmTable, globalEnums_);
        });

        forEachIntConfigKey([&, this](const IntConfigKey key) {
            readGlobalVariableConfigs_(key, yastmTable, globalInts_);
        });
    } catch (const toml::parse_error& error) {
        LOG_WARN_FMT(
            "Error while parsing general configuration file \"{}\": {}",
            configPathStr,
            error.what());
    }

    // Print the loaded configuration (we can't read the in-game forms yet.
    // Game hasn't fully initialized.)
    LOG_INFO("Loaded configuration from TOML:");

    printGlobalForms_(globalBools_);
    printGlobalForms_(globalEnums_);
    printGlobalForms_(globalInts_);
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
                    "Found YASTM soul gem configuration file: {}",
                    fileNameStr);
                configPaths.emplace_back(entry.path());
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
                "Reading individual configuration file: {}",
                configPathStr);

            validSoulGemGroupsCount += readAndCountSoulGemGroupConfigs_(table);
        } catch (const toml::parse_error& error) {
            LOG_WARN_FMT(
                "Error while parsing individual configuration file \"{}\": {}",
                configPathStr,
                error.what());
        }
    }

    // Print the loaded configuration (we can't read the in-game forms yet.
    // Game hasn't fully initialized.)
    LOG_INFO("Loaded soul gem configuration from TOML:");

    for (const auto& soulGemGroup : soulGemGroupList_) {
        LOG_INFO_FMT(
            "    {} (isReusable={}, capacity={}, priority={})",
            soulGemGroup.id(),
            soulGemGroup.isReusable(),
            soulGemGroup.capacity(),
            toString(soulGemGroup.rawPriority()));

        for (const auto& soulGemLocator : soulGemGroup.members()) {
            std::visit(
                [](auto&& soulGemLocator) {
                    LOG_INFO_FMT("        {}", soulGemLocator);
                },
                soulGemLocator);
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
            LOG_WARN_FMT("{}", issueIfMissing);
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
    for (auto& [key, globalInt] : globalInts_) { globalInt.clear(); }

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

    LOG_INFO("Loading global variable forms...");
    loadGlobalFormsIn_(globalBools_, dataHandler);
    loadGlobalFormsIn_(globalEnums_, dataHandler);
    loadGlobalFormsIn_(globalInts_, dataHandler);

    LOG_INFO("Listing loaded global variable forms:");
    printLoadedGlobalForms_(globalBools_);
    printLoadedGlobalForms_(globalEnums_);
    printLoadedGlobalForms_(globalInts_);
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

void YASTMConfig::Snapshot::printValues_() const
{
#if !defined(NDEBUG)
    LOG_TRACE("Found configuration:");

    forEachBoolConfigKey([&](const BoolConfigKey key) {
        LOG_TRACE_FMT(
            "- {}: {}",
            key,
            configBools_[static_cast<std::size_t>(key)]);
    });

    forEachEnumConfigKey([&](const EnumConfigKey key) {
        LOG_TRACE_FMT("- {}: {}", key, toString(configEnums_.at(key), key));
    });

    forEachIntConfigKey([this](const IntConfigKey key) {
        LOG_TRACE_FMT("- {}: {}", key, configInts_.at(key));
    });
#endif // !defined(NDEBUG)
}

void YASTMConfig::Snapshot::printValues_(
    const decltype(configBools_)& overrideBools,
    const decltype(configEnums_)& overrideEnums) const
{
    using BC = BoolConfigKey;
    using EC = EnumConfigKey;
    using IC = IntConfigKey;

#if !defined(NDEBUG)
    LOG_TRACE("Found configuration:");

    forEachBoolConfigKey([&](const BC key) {
        const auto oldValue = configBools_[static_cast<std::size_t>(key)];
        const auto newValue = overrideBools[static_cast<std::size_t>(key)];

        if (oldValue == newValue) {
            LOG_TRACE_FMT("- {}: {}", key, oldValue);
        } else {
            LOG_TRACE_FMT("- {}: {} (effective: {})", key, oldValue, newValue);
        }
    });

    forEachEnumConfigKey([&](const EC key) {
        const auto oldValue = configEnums_.at(key);
        const auto newValue = overrideEnums.at(key);

        if (oldValue == newValue) {
            LOG_TRACE_FMT("- {}: {}", key, toString(oldValue, key));
        } else {
            LOG_TRACE_FMT(
                "- {}: {} (effective: {})",
                key,
                toString(oldValue, key),
                toString(newValue, key));
        }
    });

    forEachIntConfigKey([this](const IC key) {
        LOG_TRACE_FMT("- {}: {}", key, configInts_.at(key));
    });
#endif // !defined(NDEBUG)
}

void YASTMConfig::Snapshot::initialize_(const YASTMConfig& config)
{
    forEachBoolConfigKey([&, this](const BoolConfigKey key) {
        configBools_[static_cast<std::size_t>(key)] = config.getGlobalBool(key);
    });

    forEachEnumConfigKey([&, this](const EnumConfigKey key) {
        configEnums_.emplace(
            key,
            static_cast<EnumConfigUnderlyingType>(config.getGlobalValue(key)));
    });

    forEachIntConfigKey([&, this](const IntConfigKey key) {
        configInts_.emplace(key, config.getGlobalInt(key));
    });
}

YASTMConfig::Snapshot::Snapshot(const YASTMConfig& config)
{
    initialize_(config);
    normalize_();
    printValues_();
}

YASTMConfig::Snapshot::Snapshot(
    const YASTMConfig& config,
    const int soulTrapLevel)
{
    using BC = BoolConfigKey;
    using EC = EnumConfigKey;
    using IC = IntConfigKey;
    using UT = EnumConfigUnderlyingType;

    initialize_(config);
    normalize_();

    if (get<EC::SoulTrapLevelingType>() != SoulTrapLevelingType::None) {
#if defined(NDEBUG)
        // In release mode, we can just modify the data structures directly,
        // so we use a reference.
        auto& bools = configBools_;
        auto& enums = configEnums_;
#else
        // In debug mode, we copy the current config values into new data
        // structures since we want to log values before/after.
        decltype(configBools_) bools(configBools_);
        decltype(configEnums_) enums(configEnums_);
#endif

        if (soulTrapLevel < configInts_[IC::SoulTrapThresholdDisplacement]) {
            bools[static_cast<std::size_t>(BC::AllowSoulDisplacement)] = false;
        }

        if (soulTrapLevel < configInts_[IC::SoulTrapThresholdRelocation]) {
            bools[static_cast<std::size_t>(BC::AllowSoulRelocation)] = false;
        }

        switch (get<EC::SoulShrinkingTechnique>()) {
        case SoulShrinkingTechnique::Shrink:
            // Shrink can't use split, so we don't care about that
            // configuration.
            if (soulTrapLevel < configInts_[IC::SoulTrapThresholdShrinking]) {
                enums[EC::SoulShrinkingTechnique] =
                    static_cast<UT>(SoulShrinkingTechnique::None);
            }
            break;
        case SoulShrinkingTechnique::Split:
            if (soulTrapLevel < configInts_[IC::SoulTrapThresholdShrinking]) {
                enums[EC::SoulShrinkingTechnique] =
                    static_cast<UT>(SoulShrinkingTechnique::None);
            } else if (
                soulTrapLevel < configInts_[IC::SoulTrapThresholdSplitting]) {
                enums[EC::SoulShrinkingTechnique] =
                    static_cast<UT>(SoulShrinkingTechnique::Shrink);
            }
            break;
        }

        printValues_(bools, enums);

#if !defined(NDEBUG)
        configBools_ = std::move(bools);
        configEnums_ = std::move(enums);
#endif // !defined(NDEBUG)
    } else {
        printValues_();
    }
}

void YASTMConfig::Snapshot::normalize_()
{
    using EC = EnumConfigKey;
    using IC = IntConfigKey;

    // Only normalize the values if we're actually going to use them.
    if (get<EC::SoulTrapLevelingType>() != SoulTrapLevelingType::None) {
        const auto normalizeValue = [this](const IC lesserKey,
                                           const IC greaterKey) {
            const auto lesserValue = configInts_[lesserKey];
            const auto greaterValue = configInts_[greaterKey];

            if (lesserValue > greaterValue) {
                LOG_WARN_FMT("{} is greater than {}", lesserKey, greaterKey);
                LOG_WARN_FMT(
                    "Setting {} to {} instead. (Currently: {})",
                    lesserKey,
                    greaterValue,
                    lesserValue);
                configInts_[lesserKey] = greaterValue;
            }
        };

        for (std::size_t i = SOULTRAP_THRESHOLD_SOULSIZE_KEYS_.size() - 1;
             i > 0;
             --i) {
            const auto currentKey = SOULTRAP_THRESHOLD_SOULSIZE_KEYS_[i];
            const auto previousKey = SOULTRAP_THRESHOLD_SOULSIZE_KEYS_[i - 1];

            normalizeValue(previousKey, currentKey);
        }

        normalizeValue(
            IC::SoulTrapThresholdDisplacement,
            IC::SoulTrapThresholdRelocation);

        normalizeValue(
            IC::SoulTrapThresholdShrinking,
            IC::SoulTrapThresholdSplitting);

        // Only normalize the values if we're actually going to use them.
        if (get<EC::SoulTrapLevelingType>() == SoulTrapLevelingType::Loss) {
            const auto scaling = configInts_[IC::SoulLossSuccessChanceScaling];

            if (scaling < 1 || scaling > 100) {
                const auto newScaling = std::clamp(scaling, 1, 100);
                LOG_WARN_FMT(
                    "{} is out of range.",
                    IC::SoulLossSuccessChanceScaling);
                LOG_WARN_FMT(
                    "Setting {} to {} instead. (Currently: {})",
                    IC::SoulLossSuccessChanceScaling,
                    newScaling,
                    scaling);
            }
        }
    }
}
