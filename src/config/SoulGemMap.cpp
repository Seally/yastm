#include "SoulGemMap.hpp"

#include <RE/F/FormTypes.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESSoulGem.h>
#include <RE/S/SoulLevels.h>

#include <fmt/format.h>

#include "../global.hpp"
#include "FormError.hpp"
#include "SpecificationError.hpp"
#include "SoulGemGroup.hpp"
#include "../utilities/TESSoulGem.hpp"
#include "../formatters/TESSoulGem.hpp"

using namespace std::literals;

std::vector<RE::TESSoulGem*> _validateAndGetForms(
    const SoulGemGroup& group,
    RE::TESDataHandler* const dataHandler)
{
    try {
        const auto reusableSoulGemKeyword = getReusableSoulGemKeyword();

        std::vector<RE::TESSoulGem*> soulGemForms;

        for (std::size_t i = 0; i < group.members().size(); ++i) {
            const auto& formId = group.members().at(i);

            const auto form =
                dataHandler->LookupForm(formId->id(), formId->pluginName());

            if (form == nullptr) {
                throw MissingFormError(*formId);
            }

            if (!form->IsSoulGem()) {
                throw UnexpectedFormTypeError(
                    RE::FormType::SoulGem,
                    form->GetFormType(),
                    form->GetName());
            }

            RE::TESSoulGem* const soulGemForm = form->As<RE::TESSoulGem>();

            // Check if the soul gem group's capacity matches the in-game version.
            //
            // We use effective capacity since black souls are grand souls
            // in-game.
            if (group.effectiveCapacity() !=
                static_cast<SoulSize>(soulGemForm->GetMaximumCapacity())) {
                throw SpecificationError(fmt::format(
                    FMT_STRING(
                        "Soul gem form {} \"{}\" in group \"{}\" does not have a capacity matching configuration."sv),
                    *formId,
                    form->GetName(),
                    group.id()));
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
                    throw FormError(fmt::format(
                        FMT_STRING(
                            "Reusable soul gem form {} \"{}\" in group \"{}\" contains a soul but has no linked soul gem specified in the form."sv),
                        *formId,
                        form->GetName(),
                        group.id()));
                }

                if (soulGemForm->linkedSoulGem->GetContainedSoul() !=
                    RE::SOUL_LEVEL::kNone) {
                    throw FormError(fmt::format(
                        FMT_STRING(
                            "Linked soul gem for reusable soul gem {} \"{}\" in group \"{}\" is not an empty soul gem."sv),
                        *formId,
                        form->GetName(),
                        group.id()));
                }
            }

            if (group.capacity() == SoulSize::Black) {
                switch (i) {
                case 0:
                    if (soulGemForm->GetContainedSoul() !=
                        RE::SOUL_LEVEL::kNone) {
                        throw SpecificationError(fmt::format(
                            FMT_STRING(
                                "Black soul gem group \"{}\" member at index {} is not an empty soul gem."sv),
                            group.id(),
                            i));
                    }
                    break;
                case 1:
                    if (soulGemForm->GetContainedSoul() !=
                        RE::SOUL_LEVEL::kGrand) {
                        throw SpecificationError(fmt::format(
                            FMT_STRING(
                                "Black soul gem group \"{}\" member at index {} is not a filled soul gem."sv),
                            group.id(),
                            i));
                    }
                    break;
                default:
                    throw SpecificationError(fmt::format(
                        FMT_STRING(
                            "Extra members found in black soul gem group \"{}\""sv),
                        group.id()));
                }
            } else {
                if (static_cast<int>(soulGemForm->GetContainedSoul()) != i) {
                    throw SpecificationError(fmt::format(
                        FMT_STRING(
                            "Soul gem group \"{}\" member at index {} does not contain the appropriate soul size."sv),
                        group.id(),
                        i));
                }
            }

            soulGemForms.push_back(soulGemForm);
        }

        return soulGemForms;
    } catch (...) {
        std::throw_with_nested(SoulGemGroupError(
            group.id(),
            fmt::format(
                FMT_STRING(
                    "Error while adding soul gem group \"{}\" to the map:"sv),
                group.id())));
    }
}

void SoulGemMap::addSoulGemGroup(
    const SoulGemGroup& group,
    RE::TESDataHandler* dataHandler)
{
    if (!_areListsInitialized) {
        initializeLists();
    }

    const auto soulGemForms = _validateAndGetForms(group, dataHandler);

    if (group.capacity() == SoulSize::Black) {
        _blackSoulGemsEmpty.push_back(soulGemForms.at(0));
        _blackSoulGemsFilled.push_back(soulGemForms.at(1));
    } else {
        for (std::size_t i = 0; i < soulGemForms.size(); ++i) {
            const auto soulGemForm = soulGemForms.at(i);

            _whiteSoulGems[group.capacity() - 1][i].push_back(soulGemForm);
        }
    }
}

const std::vector<RE::TESSoulGem*>& SoulGemMap::getSoulGemsWith(
    SoulSize capacity,
    SoulSize containedSoulSize) const
{
    using namespace std::literals;

    if (!isValidSoulCapacity(capacity) ||
        !isValidContainedSoulSize(capacity, containedSoulSize)) {
        throw InvalidSoulSpecificationError(capacity, containedSoulSize);
    }

    if (capacity == SoulSize::Black) {
        if (containedSoulSize == SoulSize::None) {
            return _blackSoulGemsEmpty;
        } else if (containedSoulSize == SoulSize::Black) {
            return _blackSoulGemsFilled;
        }
    }

    return _whiteSoulGems[capacity - 1]
                         [static_cast<std::size_t>(containedSoulSize)];
}

void SoulGemMap::initializeLists()
{
    for (std::size_t i = 0; i < _whiteSoulGems.size(); ++i) {
        _whiteSoulGems[i].resize(
            getVariantCountForCapacity(static_cast<SoulSize>(i + 1)));
    }

    _areListsInitialized = true;
}

void SoulGemMap::printContents() const
{
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
