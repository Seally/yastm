#include "GlobalVariable.hpp"

#include <RE/F/FormTypes.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESDataHandler.h>

#include "../global.hpp"
#include "FormError.hpp"

void GlobalVariable::setFromToml(const toml::array& arr)
{
    _formId.emplace(arr);
}

void GlobalVariable::loadForm(RE::TESDataHandler* const dataHandler)
{
    using namespace std::literals;

    if (!_formId.has_value()) {
        return;
    }

    const auto& formId = _formId.value();
    auto form = dataHandler->LookupForm(formId.id(), formId.pluginName());

    if (form == nullptr) {
        throw MissingFormError(formId);
    }

    const auto formType = form->GetFormType();

    if (formType == RE::FormType::Global) {
        _form = form->As<RE::TESGlobal>();
    } else {
        throw UnexpectedFormTypeError{
            RE::FormType::Global,
            formType,
            form->GetName()};
    }
}

float GlobalVariable::value() const
{
    using namespace std::literals;

    if (isFormLoaded()) {
        return _form->value;
    }

    LOG_INFO_FMT(
        "Form for {} not loaded. Returning default value."sv,
        toString(key()));
    return _defaultValue;
}
