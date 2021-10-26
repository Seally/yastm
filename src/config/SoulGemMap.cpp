#include "SoulGemMap.hpp"

#include <unordered_map>

#include <RE/F/FormTypes.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESSoulGem.h>
#include <RE/S/SoulLevels.h>

#include <fmt/format.h>

#include "../global.hpp"
#include "FormError.hpp"
#include "SoulGemGroup.hpp"
#include "../utilities/printerror.hpp"
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
                dataHandler->LookupForm(formId.id(), formId.pluginName());

            if (form == nullptr) {
                throw MissingFormError(formId);
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
                soulGemForm->GetMaximumCapacity()) {
                throw SpecificationError(fmt::format(
                    FMT_STRING(
                        "Soul gem form {} \"{}\" in group \"{}\" does not have a capacity matching configuration."sv),
                    formId,
                    form->GetName(),
                    group.id()));
            }

            const bool isReusableSoulGem =
                soulGemForm->HasKeyword(reusableSoulGemKeyword);

            // These errors aren't critical so we won't bail, but log a warning
            // about it anyway.
            if (group.isReusable()) {
                if (!isReusableSoulGem) {
                    LOG_WARN_FMT(
                        "Non-reusable soul gem {} \"{}\" is listed in reusable soul gem group \"{}\"."sv,
                        *soulGemForm,
                        soulGemForm->GetName(),
                        group.id());
                }
            } else {
                if (isReusableSoulGem) {
                    LOG_WARN_FMT(
                        "Reusable soul gem {} \"{}\" is listed in non-reusable soul gem group \"{}\""sv,
                        *soulGemForm,
                        soulGemForm->GetName(),
                        group.id());
                }
            }

            // Checks reusable soul gems for the appropriate fields.
            //
            // We use the linked soul gem field to fix a crash that occurs when
            // trying to use reusable soul gems whose base form does not have an
            // empty soul gem (the entire point of the ChargeItemFix and
            // EnchantItemFix) so it is absolutely important to get this right.
            if (isReusableSoulGem &&
                soulGemForm->GetContainedSoul() != RE::SOUL_LEVEL::kNone) {
                if (soulGemForm->linkedSoulGem == nullptr) {
                    throw FormError(fmt::format(
                        FMT_STRING(
                            "Reusable soul gem form {} \"{}\" in group \"{}\" contains a soul but has no linked soul gem specified in the form."sv),
                        formId,
                        form->GetName(),
                        group.id()));
                }

                if (soulGemForm->linkedSoulGem->GetContainedSoul() !=
                    RE::SOUL_LEVEL::kNone) {
                    throw FormError(fmt::format(
                        FMT_STRING(
                            "Linked soul gem for reusable soul gem {} \"{}\" in group \"{}\" is not an empty soul gem."sv),
                        formId,
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
        std::throw_with_nested(SoulGemGroupError(fmt::format(
            FMT_STRING(
                "Error while adding soul gem group \"{}\" to the map:"sv),
            group.id())));
    }
}

void SoulGemMap::initializeWith(
    RE::TESDataHandler* dataHandler,
    const std::function<void(Transaction&)>& fn)
{
    for (std::size_t i = 0; i < _whiteSoulGems.size(); ++i) {
        _whiteSoulGems[i].clear();
        _whiteSoulGems[i].resize(i + 2);
    }

    Transaction t;

    fn(t);

    /**
     * @brief Stores a map of the empty soul gem FormId to its soul gem group. 
    */
    std::unordered_map<
        std::reference_wrapper<const FormId>,
        std::reference_wrapper<const SoulGemGroup>,
        std::hash<FormId>>
        blackSoulGemGroupMap;

    /**
     * @brief Maps black soul gem groups to the pointer to its filled soul gem
     * form. 
     */
    std::unordered_map<const SoulGemGroup*, RE::TESSoulGem*>
        blackSoulGemFilledMap;

    for (const auto& group : t._groupsToAdd) {
        // Add black soul gems to the map.
        if (group.get().capacity() == SoulSize::Black) {
            blackSoulGemGroupMap.emplace(group.get().emptyMember(), group);
        }
    }

    const auto isDualSoulGemGroup = [&](const SoulGemGroup& group) {
        return group.capacity() == SoulSize::Grand &&
               blackSoulGemGroupMap.contains(group.emptyMember());
    };

    const auto addSoulGemGroup = [&, this](const SoulGemGroup& group) {
        try {
            const auto capacity = group.capacity();

            if (capacity == SoulSize::Black) {
                std::vector forms = _validateAndGetForms(group, dataHandler);

                const auto emptySoulGem = forms.at(0);
                const auto filledSoulGem = forms.at(1);

                _pureBlackSoulGemsEmpty.push_back(emptySoulGem);
                _pureBlackSoulGemsFilled.push_back(filledSoulGem);

                // Add the max-filled black soul gem form to the map for
                // reference when adding dual soul gems.
                blackSoulGemFilledMap.emplace(&group, filledSoulGem);
            } else if (!isDualSoulGemGroup(group)) {
                std::vector forms = _validateAndGetForms(group, dataHandler);

                for (std::size_t i = 0; i < forms.size(); ++i) {
                    const auto form = forms.at(i);

                    _whiteSoulGems[group.capacity() - 1][i].push_back(form);
                }
            }
        } catch (const std::exception& error) {
            printError(error);
        }
    };

    const auto addDualSoulGemGroup = [&, this](const SoulGemGroup& group) {
        try {
            if (isDualSoulGemGroup(group)) {
                std::vector forms = _validateAndGetForms(group, dataHandler);

                const auto& blackSoulGemGroup =
                    blackSoulGemGroupMap.at(group.emptyMember()).get();

                // If the max-filled member refers to the same soul gem form,
                // throw an error.
                if (group.filledMember() == blackSoulGemGroup.filledMember()) {
                    throw std::runtime_error(fmt::format(
                        FMT_STRING(
                            "Dual soul gem group \"{}\" max-filled member matches that of \"{}\" and cannot be disambiguated."sv),
                        group.id(),
                        blackSoulGemGroup.id()));
                }

                // If we can't find the black soul gem group in the map for the
                // max-filled black soul gem, throw an error.
                //
                // Since we should have processed all the black soul gem groups
                // already, each of which should have added an entry to this
                // map, this suggests that there's a bug.
                if (!blackSoulGemFilledMap.contains(&blackSoulGemGroup)) {
                    throw std::runtime_error(fmt::format(
                        FMT_STRING(
                            "Failed to find filled black soul gem for dual soul gem group \"{}\"."sv),
                        group.id()));
                }

                forms.push_back(blackSoulGemFilledMap.at(&blackSoulGemGroup));

                auto& dualSoulGems = _whiteSoulGems[SoulSize::Black - 1];

                for (std::size_t i = 0; i < forms.size(); ++i) {
                    const auto form = forms.at(i);

                    dualSoulGems[i].push_back(form);
                }
            }
        } catch (const std::exception& error) {
            printError(error);
        }
    };

    const auto addSoulGemGroupsForPriority =
        [&, this](
            const LoadPriority priority,
            std::function<void(const SoulGemGroup&)> addSoulGemGroup) {
            for (const auto& groupRef : t._groupsToAdd) {
                const auto& group = groupRef.get();

                if (group.priority() == priority) {
                    LOG_TRACE_FMT(
                        "Adding group \"{}\" (priority={}, rawPriority={})"sv,
                        group.id(),
                        group.priority(),
                        group.rawPriority());
                    addSoulGemGroup(group);
                }
            }
        };

    // Dual soul gems, compared to standard white grand soul gems, have an extra
    // max-filled black soul gem form. This form is used by the trap soul
    // algorithm when it looks for dual soul gems when trapping a black soul.
    //
    // In order to add this, we need the RE::TESSoulGem* loaded by the original
    // black soul gem, so we add all the non-dual soul gem groups first before
    // going over the dual soul gems.
    forEachLoadPriority([&, this](const LoadPriority priority) {
        addSoulGemGroupsForPriority(priority, addSoulGemGroup);
    });

    forEachLoadPriority([&, this](const LoadPriority priority) {
        addSoulGemGroupsForPriority(priority, addDualSoulGemGroup);
    });
}

void SoulGemMap::printContents() const
{
    for (std::size_t capacityIndex = 0; capacityIndex < _whiteSoulGems.size();
         ++capacityIndex) {
        const std::size_t soulCapacity = capacityIndex + 1;

        for (std::size_t containedSoulSize = 0;
             containedSoulSize < _whiteSoulGems[capacityIndex].size();
             ++containedSoulSize) {
            if (soulCapacity == SoulSize::Black) {
                LOG_INFO_FMT(
                    "Listing mapped dual soul gems with containedSoulSize={}"sv,
                    containedSoulSize);
            } else {
                LOG_INFO_FMT(
                    "Listing mapped soul gems with capacity={} "
                    "containedSoulSize={}",
                    soulCapacity,
                    containedSoulSize);
            }

            for (const auto soulGemForm :
                 _whiteSoulGems[capacityIndex][containedSoulSize]) {
                LOG_INFO_FMT("- {}"sv, *soulGemForm);
            }
        }
    }

    LOG_INFO("Listing mapped empty black soul gems."sv);
    for (const auto soulGemForm : _pureBlackSoulGemsEmpty) {
        LOG_INFO_FMT("- {}"sv, *soulGemForm);
    }

    LOG_INFO("Listing mapped filled black soul gems."sv);
    for (const auto soulGemForm : _pureBlackSoulGemsFilled) {
        LOG_INFO_FMT("- {}"sv, *soulGemForm);
    }
}
