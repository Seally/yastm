#include "SoulGemConfig.hpp"

#include <unordered_set>
#include <toml++/toml.h>

#include <SKSE/SKSE.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESSoulGem.h>

#include "SoulGemGroup.hpp"

void SoulGemConfig::_readConfigFromFilesystem() {
    using namespace std::literals;
    namespace logger = SKSE::log;

    std::vector<std::filesystem::path> configPaths;

    for (const auto& entry : std::filesystem::directory_iterator("Data/"sv)) {
        if (entry.exists() && !entry.path().empty() && entry.path().extension() == ".toml"sv) {
            const auto fileName = entry.path().filename();
            const auto fileNameStr = fileName.string();

            if (fileNameStr.starts_with("YASTM_"sv)) {
                logger::info("Found YASTM configuration file: {}"sv, fileNameStr);
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
                            _soulGemGroups.push_back(std::make_shared<SoulGemGroup>(SoulGemGroup::constructFromToml(el)));
                        } else {
                            throw std::runtime_error{"Value of key 'soulGems' must be a table."};
                        }
                    });
                }

                // If we made it here without an error thrown, it's a valid configuration.
                ++validConfigCount;
            }
        } catch (const toml::parse_error&) {
            logger::warn("Error while parsing config file \"{}\""sv, configPathStr);
        }
    }

    // Print the loaded configuration (we can't read the in-game forms yet. Game hasn't fully initialized.)
    logger::trace("Loaded configuration:"sv);

    for (const auto& soulGemGroup : _soulGemGroups) {
        logger::trace("    {} (isReusable={}, capacity={})"sv, soulGemGroup->id(), soulGemGroup->isReusable(), soulGemGroup->capacity());

        for (const auto& soulGemId : soulGemGroup->members()) {
            logger::trace("        [{:#08x}, {}]"sv, soulGemId->formId(), soulGemId->pluginName());
        }
    }

    if (validConfigCount <= 0) {
        throw std::runtime_error{"No valid configuration files found."};
    }
}

bool _canHoldBlackSoul(RE::TESSoulGem* const soulGemForm) {
    return soulGemForm->GetFormFlags() & RE::TESSoulGem::RecordFlags::kCanHoldNPCSoul;
}

bool SoulGemConfig::_isValidConfig(RE::TESDataHandler* const dataHandler) const {
    using namespace std::literals;
    namespace logger = SKSE::log;

    logger::info("Checking forms in configuration file...");

    for (const auto& soulGemGroup : _soulGemGroups) {
        for (int i = 0; i < soulGemGroup->members().size(); ++i) {
            auto& soulGemId = soulGemGroup->members()[i];

            RE::TESForm* const form = dataHandler->LookupForm(soulGemId->formId(), soulGemId->pluginName());

            if (form == nullptr) {
                logger::error("Form with ID {:08x} does not exist in file \"{}\""sv, soulGemId->formId(), soulGemId->pluginName());
                return false;
            }

            if (!form->IsSoulGem()) {
                logger::error("Form {:08x} \"{}\" from file \"{}\" is not a soul gem."sv, form->GetFormID(), form->GetName(), soulGemId->pluginName());
                return false;
            }

            RE::TESSoulGem* const soulGemForm = form->As<RE::TESSoulGem>();

            // We use effective capacity since black souls are grand souls in-game.
            if (soulGemGroup->effectiveCapacity() != static_cast<SoulSize>(soulGemForm->GetMaximumCapacity())) {
                logger::error("Soul gem {:08x} \"{}\" from file \"{}\" listed in group '{}' does not have a capacity matching configuration."sv, form->GetFormID(), form->GetName(), soulGemGroup->id(), soulGemId->pluginName());
                return false;
            }

            // TODO: Check NAM0 field for reusable soul gems.

            if (soulGemGroup->capacity() == SoulSize::Black) {
                switch (i) {
                case 0:
                    if (soulGemForm->GetContainedSoul() != RE::SOUL_LEVEL::kNone) {
                        logger::error("Black soul gem group \"{}\" member at index {} is not an empty soul gem."sv, soulGemGroup->id(), i);
                        return false;
                    }
                    break;
                case 1:
                    if (soulGemForm->GetContainedSoul() != RE::SOUL_LEVEL::kGrand) {
                        logger::error("Black soul gem group \"{}\" member at index {} is not a filled soul gem."sv, soulGemGroup->id(), i);
                        return false;
                    }
                    break;
                default:
                    logger::error("Extra members found in black soul gem group \"{}\"", soulGemGroup->id());
                    return false;
                }
            } else {
                if (static_cast<int>(soulGemForm->GetContainedSoul()) != i) {
                    logger::error("Soul gem group \"{}\" member at index {} does not contain the appropriate soul size."sv, soulGemGroup->id(), i);
                    return false;
                }
            }

            logger::info("Loaded form: {:08x} {}"sv, form->GetFormID(), form->GetName());
        }
    }

    return true;
}

bool SoulGemConfig::loadConfig() {
    namespace logger = SKSE::log;

    try {
        _readConfigFromFilesystem();
        return true;
    } catch (const std::exception& error) {
        logger::error(error.what());
    }

    return false;
}

void SoulGemConfig::createSoulGemMap(RE::TESDataHandler* const dataHandler) {
    if (_isValidConfig(dataHandler)) {
        _createSoulGemMap(dataHandler);
    }
}

RE::TESSoulGem* _getFormFromId(SoulGemId* const soulGemId, RE::TESDataHandler* const dataHandler) {
    return dataHandler->LookupForm<RE::TESSoulGem>(soulGemId->formId(), soulGemId->pluginName());
}

void SoulGemConfig::_createSoulGemMap(RE::TESDataHandler* const dataHandler) {
    for (int i = 0; i < _whiteSoulGems.size(); ++i) {
        _whiteSoulGems[i].resize(getVariantCountForCapacity(static_cast<SoulSize>(i + 1)));
    }

    for (const auto& soulGemGroup : _soulGemGroups) {
        if (soulGemGroup->isReusable()) {
            if (soulGemGroup->capacity() == SoulSize::Black) {
                const auto emptySoulGemForm = _getFormFromId(soulGemGroup->members()[0].get(), dataHandler);
                const auto filledSoulGemForm = _getFormFromId(soulGemGroup->members()[1].get(), dataHandler);

                _blackSoulGemsEmpty.push_back(emptySoulGemForm);
                _blackSoulGemsFilled.push_back(filledSoulGemForm);
            } else {
                for (int i = 0; i < soulGemGroup->members().size(); ++i) {
                    const auto soulGemForm = _getFormFromId(soulGemGroup->members()[i].get(), dataHandler);

                    _whiteSoulGems[static_cast<std::size_t>(soulGemGroup->capacity()) - 1][i].push_back(soulGemForm);
                }
            }
        }
    }

    for (const auto& soulGemGroup : _soulGemGroups) {
        if (!soulGemGroup->isReusable()) {
            if (soulGemGroup->capacity() == SoulSize::Black) {
                const auto emptySoulGemForm = _getFormFromId(soulGemGroup->members()[0].get(), dataHandler);
                const auto filledSoulGemForm = _getFormFromId(soulGemGroup->members()[1].get(), dataHandler);

                _blackSoulGemsEmpty.push_back(emptySoulGemForm);
                _blackSoulGemsFilled.push_back(filledSoulGemForm);
            } else {
                for (int i = 0; i < soulGemGroup->members().size(); ++i) {
                    const auto soulGemForm = _getFormFromId(soulGemGroup->members()[i].get(), dataHandler);

                    _whiteSoulGems[static_cast<std::size_t>(soulGemGroup->capacity()) - 1][i].push_back(soulGemForm);
                }
            }
        }
    }

    namespace logger = SKSE::log;
    using namespace std::literals;

    for (int i = 0; i < _whiteSoulGems.size(); ++i) {
        const int soulCapacity = i + 1;

        for (int containedSoulSize = 0; containedSoulSize < _whiteSoulGems[i].size(); ++containedSoulSize) {
            logger::info("Listing mapped soul gems with capacity={} containedSoulSize={}", soulCapacity, containedSoulSize);

            for (const auto soulGemForm : _whiteSoulGems[i][containedSoulSize]) {
                logger::info("- [ID:{:08x}] {} (capacity={}, containedSoulSize={}, canHoldBlackSoul={})"sv, soulGemForm->GetFormID(), soulGemForm->GetName(), soulGemForm->GetMaximumCapacity(), soulGemForm->GetContainedSoul(), _canHoldBlackSoul(soulGemForm));
            }
        }
    }

    logger::info("Listing mapped empty black soul gems.");
    for (const auto soulGemForm : _blackSoulGemsEmpty) {
        logger::info("- [ID:{:08x}] {} (capacity={}, containedSoulSize={}, canHoldBlackSoul={})"sv, soulGemForm->GetFormID(), soulGemForm->GetName(), soulGemForm->GetMaximumCapacity(), soulGemForm->GetContainedSoul(), _canHoldBlackSoul(soulGemForm));
    }

    logger::info("Listing mapped filled black soul gems.");
    for (const auto soulGemForm : _blackSoulGemsEmpty) {
        logger::info("- [ID:{:08x}] {} (capacity={}, containedSoulSize={}, canHoldBlackSoul={})"sv, soulGemForm->GetFormID(), soulGemForm->GetName(), soulGemForm->GetMaximumCapacity(), soulGemForm->GetContainedSoul(), _canHoldBlackSoul(soulGemForm));
    }
}

const std::vector<RE::TESSoulGem*>& SoulGemConfig::getSoulGemsWith(const SoulSize soulCapacity, const SoulSize containedSoulSize) const {
    using namespace std::literals;

    if (!isValidSoulCapacity(soulCapacity)) {
        throw std::range_error(std::format("Attempting to lookup invalid soul capacity: {}"sv, static_cast<int>(soulCapacity)));
    }

    if (!isValidContainedSoulSize(soulCapacity, containedSoulSize)) {
        throw std::range_error(std::format("Attempting to lookup invalid contained soul size {} for capacity {}"sv, static_cast<int>(containedSoulSize), static_cast<int>(soulCapacity)));
    }

    if (soulCapacity == SoulSize::Black) {
        if (containedSoulSize == SoulSize::None) {
            return _blackSoulGemsEmpty;
        } else if (containedSoulSize == SoulSize::Black) {
            return _blackSoulGemsFilled;
        }
    } else {
        return _whiteSoulGems[static_cast<std::size_t>(soulCapacity) - 1][containedSoulSize];
    }

    throw std::range_error(std::format("Attempting to lookup invalid contained soul size {} for capacity {}"sv, static_cast<int>(containedSoulSize), static_cast<int>(soulCapacity)));
}
