#include "ConcreteSoulGemGroup.hpp"

#include <cassert>

#include <fmt/format.h>

#include "../global.hpp"
#include "FormError.hpp"
#include "SpecificationError.hpp"
#include "../formatters/TESSoulGem.hpp"
#include "../utilities/misc.hpp"
#include "../utilities/native.hpp"

namespace {
    SoulSize toContainedSoulSize_(
        const SoulGemCapacity capacity,
        const std::size_t index)
    {
        if (capacity == SoulGemCapacity::Black) {
            switch (index) {
            case 0:
                return SoulSize::None;
            case 1:
                return SoulSize::Black;
            }
        } else {
            switch (index) {
            case 0:
                return SoulSize::None;
            case 1:
                return SoulSize::Petty;
            case 2:
                return SoulSize::Lesser;
            case 3:
                return SoulSize::Common;
            case 4:
                return SoulSize::Greater;
            case 5:
                return SoulSize::Grand;
            }
        }

        throw std::runtime_error(fmt::format(
            FMT_STRING("Invalid member index {} for capacity {}"),
            index,
            capacity));
    }

    void checkFormIsNotNull_(RE::TESForm* const form, const FormId& formId)
    {
        if (form == nullptr) {
            throw MissingFormError(formId);
        }
    }

    RE::TESSoulGem* toSoulGem_(RE::TESForm* const form)
    {
        const auto soulGemForm = form->As<RE::TESSoulGem>();

        if (soulGemForm == nullptr) {
            throw UnexpectedFormTypeError(
                RE::FormType::SoulGem,
                form->GetFormType(),
                form->GetName());
        }

        return soulGemForm;
    }

    void checkGroupCapacityMatchesSoulGemFormCapacity_(
        RE::TESSoulGem* const form,
        const FormId& formId,
        const SoulGemGroup& group)
    {
        // We use effective capacity since black souls are grand souls
        // in-game.
        if (group.effectiveCapacity() != form->GetMaximumCapacity()) {
            throw SpecificationError(fmt::format(
                FMT_STRING(
                    "Soul gem form {} \"{}\" in {} does not have a capacity matching configuration"sv),
                formId,
                form->GetName(),
                group));
        }
    }

    bool checkSoulGemReusability_(
        RE::TESSoulGem* const soulGemForm,
        const SoulGemGroup& group)
    {
        const bool isReusable =
            soulGemForm->HasKeyword(getReusableSoulGemKeyword());

        // These errors aren't critical so we won't bail, but log a warning
        // about it anyway.
        if (group.isReusable()) {
            if (!isReusable) {
                LOG_WARN_FMT(
                    "Non-reusable soul gem {} is listed in {:r}"sv,
                    *soulGemForm,
                    group);
            }
        } else {
            if (isReusable) {
                LOG_WARN_FMT(
                    "Reusable soul gem {} is listed in {:r}"sv,
                    *soulGemForm,
                    group);
            }
        }

        return isReusable;
    }

    void checkReusableSoulGemFields_(
        RE::TESSoulGem* const soulGemForm,
        const SoulGemGroup& group)
    {
        // Checks reusable soul gems for the appropriate fields.
        //
        // We use the linked soul gem field to fix a crash that occurs when
        // trying to use reusable soul gems whose base form does not have an
        // empty soul gem (the entire point of the ChargeItemFix and
        // EnchantItemFix) so it is absolutely important to get this right.
        if (soulGemForm->GetContainedSoul() != RE::SOUL_LEVEL::kNone) {
            if (soulGemForm->linkedSoulGem == nullptr) {
                throw FormError(fmt::format(
                    FMT_STRING(
                        "Reusable soul gem {} in {} contains a soul but has "
                        "no linked soul gem specified in the form."),
                    *soulGemForm,
                    group));
            }

            if (soulGemForm->linkedSoulGem->GetContainedSoul() !=
                RE::SOUL_LEVEL::kNone) {
                throw FormError(fmt::format(
                    FMT_STRING(
                        "Linked soul gem for reusable soul gem {} in {} is "
                        "not an empty soul gem."),
                    *soulGemForm,
                    group));
            }
        }
    }

    void checkIndexMatchesContainedSoulSize_(
        const std::size_t index,
        RE::TESSoulGem* const soulGemForm,
        const SoulGemGroup& group)
    {
        if (group.capacity() == SoulGemCapacity::Black) {
            switch (index) {
            case 0:
                if (soulGemForm->GetContainedSoul() != RE::SOUL_LEVEL::kNone) {
                    throw SpecificationError(fmt::format(
                        FMT_STRING("{:uc} member at index {} is not an empty "
                                   "soul gem."),
                        group,
                        index));
                }
                break;
            case 1:
                if (soulGemForm->GetContainedSoul() != RE::SOUL_LEVEL::kGrand) {
                    throw SpecificationError(fmt::format(
                        FMT_STRING("{:uc} member at index {} is not a filled "
                                   "soul gem."),
                        group,
                        index));
                }
                break;
            default:
                throw SpecificationError(fmt::format(
                    FMT_STRING("Extra members found in {:c}"),
                    group));
            }
        } else if (static_cast<int>(soulGemForm->GetContainedSoul()) != index) {
            throw SpecificationError(fmt::format(
                FMT_STRING(
                    "{:u} member at index {} does not contain the appropriate soul size."sv),
                group,
                index));
        }
    }
} // namespace

void ConcreteSoulGemGroup::initializeFromPrimaryBasis_(
    const SoulGemGroup& sourceGroup,
    RE::TESDataHandler* const dataHandler)
{
    capacity_ = sourceGroup.capacity();

    for (std::size_t i = 0; i < sourceGroup.members().size(); ++i) {
        const auto& formId = sourceGroup.members().at(i);

        const auto form =
            dataHandler->LookupForm(formId.id(), formId.pluginName());

        checkFormIsNotNull_(form, formId);
        const auto soulGemForm = toSoulGem_(form);

        checkGroupCapacityMatchesSoulGemFormCapacity_(
            soulGemForm,
            formId,
            sourceGroup);

        const bool isReusable =
            checkSoulGemReusability_(soulGemForm, sourceGroup);
        if (isReusable) {
            checkReusableSoulGemFields_(soulGemForm, sourceGroup);
        }

        forms_.emplace(
            toContainedSoulSize_(sourceGroup.capacity(), i),
            soulGemForm);
    }
}

void ConcreteSoulGemGroup::initializeFromSecondaryBasis_(
    const ConcreteSoulGemGroup& blackSoulGemGroup)
{
    if (at(SoulSize::None) != blackSoulGemGroup.at(SoulSize::None)) {
        throw std::runtime_error(fmt::format(
            FMT_STRING("{:c} and {:c} do not have the same empty form."),
            *this,
            blackSoulGemGroup));
    }

    if (at(SoulSize::Grand) == blackSoulGemGroup.at(SoulSize::Black)) {
        throw std::runtime_error(fmt::format(
            FMT_STRING("{:c} and {:c} share the same fully-filled form and "
                       "cannot be disambiguated."),
            *this,
            blackSoulGemGroup));
    }

    if (forms_.contains(SoulSize::Black)) {
        throw std::runtime_error(fmt::format(
            FMT_STRING("{:c} already contains a black soul gem member."),
            *this));
    }

    forms_.emplace(SoulSize::Black, blackSoulGemGroup.at(SoulSize::Black));
    capacity_ = SoulGemCapacity::Dual;
}

ConcreteSoulGemGroup::ConcreteSoulGemGroup(
    const SoulGemGroup& sourceGroup,
    RE::TESDataHandler* const dataHandler)
{
    try {
        initializeFromPrimaryBasis_(sourceGroup, dataHandler);
    } catch (...) {
        std::throw_with_nested(ConcreteSoulGemGroupError(fmt::format(
            FMT_STRING("Error while creating concrete soul gem group from {}:"),
            sourceGroup)));
    }
}

ConcreteSoulGemGroup::ConcreteSoulGemGroup(
    const SoulGemGroup& whiteGrandSoulGemGroup,
    const ConcreteSoulGemGroup& blackSoulGemGroup,
    RE::TESDataHandler* const dataHandler)
{
    try {
        if (whiteGrandSoulGemGroup.capacity() != SoulGemCapacity::Grand) {
            throw std::runtime_error(fmt::format(
                FMT_STRING("Cannot create dual soul gem group with {:c} as the "
                           "primary basis."),
                whiteGrandSoulGemGroup));
        }

        if (blackSoulGemGroup.capacity() != SoulGemCapacity::Black) {
            throw std::runtime_error(fmt::format(
                FMT_STRING("Cannot create dual soul gem group with "
                           "{:c} as the secondary basis."),
                blackSoulGemGroup));
        }

        initializeFromPrimaryBasis_(whiteGrandSoulGemGroup, dataHandler);
        initializeFromSecondaryBasis_(blackSoulGemGroup);

        assert(capacity_ == SoulGemCapacity::Dual);
    } catch (...) {
        std::throw_with_nested(ConcreteSoulGemGroupError(fmt::format(
            FMT_STRING(
                "Error while creating concrete soul gem group from {:c} and {:c}:"sv),
            whiteGrandSoulGemGroup,
            blackSoulGemGroup)));
    }
}
