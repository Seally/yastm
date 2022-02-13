#include "FormError.hpp"

#include "../utilities/FormType.hpp"

using namespace std::literals;

UnexpectedFormTypeError::UnexpectedFormTypeError(
    const RE::FormType expectedFormType,
    const RE::FormType receivedFormType,
    std::string_view formName)
    : FormError(fmt::format(
          FMT_STRING("Expected form type {} but received {} \"{}\""sv),
          toString(expectedFormType),
          toString(receivedFormType),
          formName))
    , expectedFormType(expectedFormType)
    , receivedFormType(receivedFormType)
    , formName(formName)
{}
