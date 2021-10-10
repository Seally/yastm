#include "GlobalVariable.hpp"

#include <RE/F/FormTypes.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESDataHandler.h>

#include "../global.hpp"
#include "FormError.hpp"

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
