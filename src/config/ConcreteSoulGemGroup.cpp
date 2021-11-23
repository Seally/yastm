#include "ConcreteSoulGemGroup.hpp"

#include <cassert>

#include <fmt/format.h>

#include "../global.hpp"
#include "../formatters/TESSoulGem.hpp"
#include "../utilities/soultraputils.hpp"
#include "FormError.hpp"
#include "SpecificationError.hpp"

SoulSize _toContainedSoulSize(
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

void _checkFormIsNotNull(RE::TESForm* form, const FormId& formId)
{
    if (form == nullptr) {
        throw MissingFormError(formId);
    }
}

void _checkFormIsSoulGem(RE::TESForm* form)
{
    if (!form->IsSoulGem()) {
        throw UnexpectedFormTypeError(
            RE::FormType::SoulGem,
            form->GetFormType(),
            form->GetName());
    }
}

void _checkGroupCapacityMatchesSoulGemFormCapacity(
    RE::TESSoulGem* form,
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

bool _checkSoulGemReusability(
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

void _checkReusableSoulGemFields(
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
                FMT_STRING("Reusable soul gem {} in {} contains a soul but has "
                           "no linked soul gem specified in the form."),
                *soulGemForm,
                group));
        }

        if (soulGemForm->linkedSoulGem->GetContainedSoul() !=
            RE::SOUL_LEVEL::kNone) {
            throw FormError(fmt::format(
                FMT_STRING("Linked soul gem for reusable soul gem {} in {} is "
                           "not an empty soul gem."),
                *soulGemForm,
                group));
        }
    }
}

void _checkIndexMatchesContainedSoulSize(
    const std::size_t index,
    RE::TESSoulGem* const soulGemForm,
    const SoulGemGroup& group)
{
    if (group.capacity() == SoulGemCapacity::Black) {
        switch (index) {
        case 0:
            if (soulGemForm->GetContainedSoul() != RE::SOUL_LEVEL::kNone) {
                throw SpecificationError(fmt::format(
                    FMT_STRING(
                        "{:uc} member at index {} is not an empty soul gem."),
                    group,
                    index));
            }
            break;
        case 1:
            if (soulGemForm->GetContainedSoul() != RE::SOUL_LEVEL::kGrand) {
                throw SpecificationError(fmt::format(
                    FMT_STRING(
                        "{:uc} member at index {} is not a filled soul gem."),
                    group,
                    index));
            }
            break;
        default:
            throw SpecificationError(
                fmt::format(FMT_STRING("Extra members found in {:c}"), group));
        }
    } else if (static_cast<int>(soulGemForm->GetContainedSoul()) != index) {
        throw SpecificationError(fmt::format(
            FMT_STRING(
                "{:u} member at index {} does not contain the appropriate soul size."sv),
            group,
            index));
    }
}

void ConcreteSoulGemGroup::_initializeFromPrimaryBasis(
    const SoulGemGroup& sourceGroup,
    RE::TESDataHandler* dataHandler)
{
    _capacity = sourceGroup.capacity();

    for (std::size_t i = 0; i < sourceGroup.members().size(); ++i) {
        const auto& formId = sourceGroup.members().at(i);

        const auto form =
            dataHandler->LookupForm(formId.id(), formId.pluginName());

        _checkFormIsNotNull(form, formId);
        _checkFormIsSoulGem(form);

        RE::TESSoulGem* const soulGemForm = form->As<RE::TESSoulGem>();

        _checkGroupCapacityMatchesSoulGemFormCapacity(
            soulGemForm,
            formId,
            sourceGroup);

        const bool isReusable =
            _checkSoulGemReusability(soulGemForm, sourceGroup);
        if (isReusable) {
            _checkReusableSoulGemFields(soulGemForm, sourceGroup);
        }

        _forms.emplace(
            _toContainedSoulSize(sourceGroup.capacity(), i),
            soulGemForm);
    }
}

void ConcreteSoulGemGroup::_initializeFromSecondaryBasis(
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

    if (_forms.contains(SoulSize::Black)) {
        throw std::runtime_error(fmt::format(
            FMT_STRING("{:c} already contains a black soul gem member."),
            *this));
    }

    _forms.emplace(SoulSize::Black, blackSoulGemGroup.at(SoulSize::Black));
    _capacity = SoulGemCapacity::Dual;
}

ConcreteSoulGemGroup::ConcreteSoulGemGroup(
    const SoulGemGroup& sourceGroup,
    RE::TESDataHandler* dataHandler)
{
    try {
        _initializeFromPrimaryBasis(sourceGroup, dataHandler);
    } catch (...) {
        std::throw_with_nested(ConcreteSoulGemGroupError(fmt::format(
            FMT_STRING("Error while creating concrete soul gem group from {}:"),
            sourceGroup)));
    }
}

ConcreteSoulGemGroup::ConcreteSoulGemGroup(
    const SoulGemGroup& whiteGrandSoulGemGroup,
    const ConcreteSoulGemGroup& blackSoulGemGroup,
    RE::TESDataHandler* dataHandler)
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

        _initializeFromPrimaryBasis(whiteGrandSoulGemGroup, dataHandler);
        _initializeFromSecondaryBasis(blackSoulGemGroup);

        assert(_capacity == SoulGemCapacity::Dual);
    } catch (...) {
        std::throw_with_nested(ConcreteSoulGemGroupError(fmt::format(
            FMT_STRING(
                "Error while creating concrete soul gem group from {:c} and {:c}:"sv),
            whiteGrandSoulGemGroup,
            blackSoulGemGroup)));
    }
}

ConcreteSoulGemGroupError::ConcreteSoulGemGroupError(const std::string& message)
    : std::runtime_error{message}
{}
