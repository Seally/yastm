#include "YASTMConfig.hpp"

#include <filesystem>

#include <toml++/toml.h>

#include <RE/B/BGSDefaultObjectManager.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESGlobal.h>
#include <RE/T/TESSoulGem.h>
#include <SKSE/SKSE.h>

#include "../global.hpp"
#include "SoulGemGroup.hpp"
#include "../formatters/TESSoulGem.hpp"

void YASTMConfig::_readYASTMConfig()
{
    using namespace std::literals;

    toml::table table;

    const std::filesystem::path configPath{"Data/YASTM.toml"sv};
    const std::string configPathStr{configPath.string()};

    try {
        table = toml::parse_file(configPathStr);

        LOG_INFO_FMT(
            "Found YASTM general configuration file: {}"sv,
            configPath.filename().string());

        const auto yastmTable = table["YASTM"];

        const auto readIdFromToml = [&](const Key key) {
            const auto& keyName = YASTMConfig::toKeyName(key);

            if (const auto idArray =
                    yastmTable[std::string{keyName} + "Global"].as_array();
                idArray) {
                _globals.insert(std::make_pair(
                    key,
                    GlobalId::constructFromToml(keyName, *idArray)));
            } else {
                LOG_WARN_FMT(
                    "Form data for configuration key '{}' not found."sv,
                    keyName);
            }
        };

        readIdFromToml(Key::AllowPartiallyFillingSoulGems);
        readIdFromToml(Key::AllowSoulDisplacement);
        readIdFromToml(Key::AllowSoulRelocation);
        readIdFromToml(Key::AllowSoulShrinking);
        readIdFromToml(Key::AllowExtraSoulRelocation);
        readIdFromToml(Key::PreserveOwnership);
        readIdFromToml(Key::AllowNotifications);
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

    for (const auto& [key, globalId] : _globals) {
        LOG_TRACE_FMT(
            "- {} = [{:08x}, {}]"sv,
            globalId.keyName(),
            globalId.formId(),
            globalId.pluginName());
    }
#endif // NDEBUG
}

void YASTMConfig::_readSoulGemConfigs()
{
    using namespace std::literals;

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

    std::size_t validConfigCount = 0;

    for (const auto& configPath : configPaths) {
        toml::table table;

        std::string configPathStr = configPath.string();

        try {
            table = toml::parse_file(configPathStr);

            if (auto soulGems = table["soulGems"sv].as_array()) {
                for (toml::node& elem : *soulGems) {
                    elem.visit([this](auto&& el) {
                        if constexpr (toml::is_table<decltype(el)>) {
                            _soulGemGroups.push_back(
                                std::make_unique<SoulGemGroup>(
                                    SoulGemGroup::constructFromToml(el)));
                        } else {
                            throw std::runtime_error{
                                "Value of key 'soulGems' must be a table."};
                        }
                    });
                }

                // If we made it here without an error thrown, it's a valid
                // configuration.
                ++validConfigCount;
            }
        } catch (const toml::parse_error&) {
            LOG_WARN_FMT(
                "Error while parsing soul gem configuration file \"{}\""sv,
                configPathStr);
        }
    }

#ifndef NDEBUG
    // Print the loaded configuration (we can't read the in-game forms yet.
    // Game hasn't fully initialized.)
    LOG_TRACE("Loaded soul gem configuration from TOML:"sv);

    for (const auto& soulGemGroup : _soulGemGroups) {
        LOG_TRACE_FMT(
            "    {} (isReusable={}, capacity={}, priority={})"sv,
            soulGemGroup->id(),
            soulGemGroup->isReusable(),
            soulGemGroup->capacity(),
            toLoadPriorityString(soulGemGroup->rawPriority()));

        for (const auto& soulGemId : soulGemGroup->members()) {
            LOG_TRACE_FMT(
                "        [{:#08x}, {}]"sv,
                soulGemId->formId(),
                soulGemId->pluginName());
        }
    }
#endif // NDEBUG

    if (validConfigCount <= 0) {
        throw std::runtime_error{"No valid configuration files found."};
    }
}

bool YASTMConfig::_isValidConfig(RE::TESDataHandler* const dataHandler) const
{
    using namespace std::literals;

    LOG_INFO("Loading soul gem forms..."sv);

    const auto reusableSoulGemKeyword = getReusableSoulGemKeyword();

    for (const auto& soulGemGroup : _soulGemGroups) {
        for (int i = 0; i < soulGemGroup->members().size(); ++i) {
            auto& soulGemId = soulGemGroup->members()[i];

            RE::TESForm* const form = dataHandler->LookupForm(
                soulGemId->formId(),
                soulGemId->pluginName());

            if (form == nullptr) {
                LOG_ERROR_FMT(
                    "Form with ID {:08x} does not exist in file \"{}\""sv,
                    soulGemId->formId(),
                    soulGemId->pluginName());
                return false;
            }

            if (!form->IsSoulGem()) {
                LOG_ERROR_FMT(
                    "Form {:08x} \"{}\" from file \"{}\" is not a soul gem."sv,
                    form->GetFormID(),
                    form->GetName(),
                    soulGemId->pluginName());
                return false;
            }

            RE::TESSoulGem* const soulGemForm = form->As<RE::TESSoulGem>();

            // We use effective capacity since black souls are grand souls
            // in-game.
            if (soulGemGroup->effectiveCapacity() !=
                static_cast<SoulSize>(soulGemForm->GetMaximumCapacity())) {
                LOG_ERROR_FMT(
                    "Soul gem {:08x} \"{}\" from file \"{}\" in group '{}' does not have a capacity matching configuration."sv,
                    form->GetFormID(),
                    form->GetName(),
                    soulGemId->pluginName(),
                    soulGemGroup->id());
                return false;
            }

            // Checks reusable soul gems for the appropriate fields.
            //
            // We use the linked soul gem field to fix a crash that occurs when
            // trying to use reusable soul gems whose base form does not have an
            // empty soul gem (the entire point of the ChargeItemFix and
            // EnchantItemFix) so it is absolutely important to get this right.
            if (soulGemForm->HasKeyword(reusableSoulGemKeyword) &&
                soulGemForm->GetContainedSoul() != RE::SOUL_LEVEL::kNone) {
                if (soulGemForm->linkedSoulGem == nullptr) {
                    LOG_ERROR_FMT(
                        "Reusable soul gem {:08x} \"{}\" from file \"{}\" in group '{}' contains a soul but has no linked soul gem specified in the form."sv,
                        form->GetFormID(),
                        form->GetName(),
                        soulGemId->pluginName(),
                        soulGemGroup->id());
                    return false;
                }

                if (soulGemForm->linkedSoulGem->GetContainedSoul() !=
                    RE::SOUL_LEVEL::kNone) {
                    LOG_ERROR_FMT(
                        "Linked soul gem for reusable soul gem {:08x} \"{}\" from file \"{}\" in group '{}' is not an empty soul gem."sv,
                        form->GetFormID(),
                        form->GetName(),
                        soulGemId->pluginName(),
                        soulGemGroup->id());
                    return false;
                }
            }

            // TODO: Check NAM0 field for reusable soul gems.
            if (soulGemGroup->capacity() == SoulSize::Black) {
                switch (i) {
                case 0:
                    if (soulGemForm->GetContainedSoul() !=
                        RE::SOUL_LEVEL::kNone) {
                        LOG_ERROR_FMT(
                            "Black soul gem group \"{}\" member at index {} is not an empty soul gem."sv,
                            soulGemGroup->id(),
                            i);
                        return false;
                    }
                    break;
                case 1:
                    if (soulGemForm->GetContainedSoul() !=
                        RE::SOUL_LEVEL::kGrand) {
                        LOG_ERROR_FMT(
                            "Black soul gem group \"{}\" member at index {} is not a filled soul gem."sv,
                            soulGemGroup->id(),
                            i);
                        return false;
                    }
                    break;
                default:
                    LOG_ERROR_FMT(
                        "Extra members found in black soul gem group \"{}\""sv,
                        soulGemGroup->id());
                    return false;
                }
            } else {
                if (static_cast<int>(soulGemForm->GetContainedSoul()) != i) {
                    LOG_ERROR_FMT(
                        "Soul gem group \"{}\" member at index {} does not contain the appropriate soul size."sv,
                        soulGemGroup->id(),
                        i);
                    return false;
                }
            }

            LOG_INFO_FMT(
                "- Loaded form: [ID:{:08x}] {}"sv,
                form->GetFormID(),
                form->GetName());
        }
    }

    return true;
}

bool YASTMConfig::loadConfig()
{
    try {
        _readYASTMConfig();
        _readSoulGemConfigs();
        return true;
    } catch (const std::exception& error) {
        LOG_ERROR(error.what());
    }

    return false;
}

void YASTMConfig::processGameForms(RE::TESDataHandler* const dataHandler)
{
    if (_isValidConfig(dataHandler)) {
        _getGlobalForms(dataHandler);
        _createSoulGemMap(dataHandler);
    }
}

float YASTMConfig::getGlobalValue(const Key key) const
{
    if (_globals.contains(key)) {
        const auto& globalId = _globals.at(key);

        if (globalId.form() == nullptr) {
            LOG_TRACE_FMT(
                "Global variable '{}' ({}) not yet loaded. Returning default value..."sv,
                YASTMConfig::toKeyName(key),
                globalId);
            return _globalsDefaults.at(key);
        }

        return globalId.form()->value;
    }

    LOG_TRACE_FMT(
        "Global variable '{}' not specified in configuration. Returning default value..."sv,
        YASTMConfig::toKeyName(key));
    return _globalsDefaults.at(key);
}

bool YASTMConfig::isPartialFillsAllowed() const
{
    return getGlobalValue(Key::AllowPartiallyFillingSoulGems) != 0;
}

bool YASTMConfig::isSoulDisplacementAllowed() const
{
    return getGlobalValue(Key::AllowSoulDisplacement) != 0;
}

bool YASTMConfig::isSoulRelocationAllowed() const
{
    return getGlobalValue(Key::AllowSoulRelocation) != 0;
}

bool YASTMConfig::isExtraSoulRelocationAllowed() const
{
    return getGlobalValue(Key::AllowExtraSoulRelocation) != 0;
}

bool YASTMConfig::isSoulShrinkingAllowed() const
{
    return getGlobalValue(Key::AllowSoulShrinking) != 0;
}

bool YASTMConfig::preserveOwnership() const
{
    return getGlobalValue(Key::PreserveOwnership) != 0;
}

bool YASTMConfig::isNotificationsAllowed() const
{
    using namespace std::literals;

    return getGlobalValue(Key::AllowNotifications) != 0;
}

RE::TESSoulGem* _getFormFromId(
    SoulGemId* const soulGemId,
    RE::TESDataHandler* const dataHandler)
{
    return dataHandler->LookupForm<RE::TESSoulGem>(
        soulGemId->formId(),
        soulGemId->pluginName());
}

void YASTMConfig::_getGlobalForms(RE::TESDataHandler* const dataHandler)
{
    using namespace std::literals;

    LOG_INFO("Loading global variable forms..."sv);

    for (auto& [key, globalId] : _globals) {
        const auto form =
            dataHandler->LookupForm(globalId.formId(), globalId.pluginName());

        if (form->Is(RE::FormType::Global)) {
            globalId.setForm(form->As<RE::TESGlobal>());

            LOG_INFO_FMT(
                "- Loaded form: [ID:{:08x}] (key: {})"sv,
                form->GetFormID(),
                globalId.keyName());
        } else {
            LOG_ERROR_FMT(
                "Form {:08x} \"{}\" from file \"{}\" is not a global variable."sv,
                form->GetFormID(),
                form->GetName(),
                globalId.pluginName());
        }
    }
}

void YASTMConfig::_createSoulGemMap(RE::TESDataHandler* const dataHandler)
{
    using namespace std::literals;

    for (int i = 0; i < _whiteSoulGems.size(); ++i) {
        _whiteSoulGems[i].resize(
            getVariantCountForCapacity(static_cast<SoulSize>(i + 1)));
    }

    const auto addSoulGemGroupToMap = [=, this](const SoulGemGroup& group) {
        if (group.capacity() == SoulSize::Black) {
            const auto emptySoulGemForm =
                _getFormFromId(group.members()[0].get(), dataHandler);
            const auto filledSoulGemForm =
                _getFormFromId(group.members()[1].get(), dataHandler);

            _blackSoulGemsEmpty.push_back(emptySoulGemForm);
            _blackSoulGemsFilled.push_back(filledSoulGemForm);
        } else {
            for (int i = 0; i < group.members().size(); ++i) {
                const auto soulGemForm =
                    _getFormFromId(group.members()[i].get(), dataHandler);

                _whiteSoulGems[static_cast<std::size_t>(group.capacity()) - 1]
                              [i]
                                  .push_back(soulGemForm);
            }
        }
    };

    const auto createSoulGemGroupForPriority =
        [=, this](const LoadPriority priority) {
            for (const auto& soulGemGroup : _soulGemGroups) {
                if (soulGemGroup->priority() == priority) {
                    addSoulGemGroupToMap(*soulGemGroup);
                }
            }
        };

    createSoulGemGroupForPriority(LoadPriority::High);
    createSoulGemGroupForPriority(LoadPriority::Normal);
    createSoulGemGroupForPriority(LoadPriority::Low);

    for (int i = 0; i < _whiteSoulGems.size(); ++i) {
        const int soulCapacity = i + 1;

        for (int containedSoulSize = 0;
             containedSoulSize < _whiteSoulGems[i].size();
             ++containedSoulSize) {
            LOG_INFO_FMT(
                "Listing mapped soul gems with capacity={} "
                "containedSoulSize={}",
                soulCapacity,
                containedSoulSize);

            for (const auto soulGemForm :
                 _whiteSoulGems[i][containedSoulSize]) {
                LOG_INFO_FMT("- {}"sv, soulGemForm);
            }
        }
    }

    LOG_INFO("Listing mapped empty black soul gems."sv);
    for (const auto soulGemForm : _blackSoulGemsEmpty) {
        LOG_INFO_FMT("- {}"sv, soulGemForm);
    }

    LOG_INFO("Listing mapped filled black soul gems."sv);
    for (const auto soulGemForm : _blackSoulGemsEmpty) {
        LOG_INFO_FMT("- {}"sv, soulGemForm);
    }
}

const std::vector<RE::TESSoulGem*>& YASTMConfig::getSoulGemsWith(
    const SoulSize soulCapacity,
    const SoulSize containedSoulSize) const
{
    using namespace std::literals;

    if (!isValidSoulCapacity(soulCapacity)) {
        throw std::range_error(std::format(
            "Attempting to lookup invalid soul capacity: {}"sv,
            static_cast<int>(soulCapacity)));
    }

    if (!isValidContainedSoulSize(soulCapacity, containedSoulSize)) {
        throw std::range_error(std::format(
            "Attempting to lookup invalid contained soul size {} for capacity {}"sv,
            static_cast<int>(containedSoulSize),
            static_cast<int>(soulCapacity)));
    }

    if (soulCapacity == SoulSize::Black) {
        if (containedSoulSize == SoulSize::None) {
            return _blackSoulGemsEmpty;
        } else if (containedSoulSize == SoulSize::Black) {
            return _blackSoulGemsFilled;
        }
    } else {
        return _whiteSoulGems[static_cast<std::size_t>(soulCapacity) - 1]
                             [containedSoulSize];
    }

    throw std::range_error(std::format(
        "Attempting to lookup invalid contained soul size {} for capacity {}"sv,
        static_cast<int>(containedSoulSize),
        static_cast<int>(soulCapacity)));
}
