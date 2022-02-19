#pragma once

#include <exception>
#include <string_view>

#include <fmt/format.h>

#include <RE/F/FormTypes.h>

#include "FormId.hpp"

class FormError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class UnexpectedFormTypeError : public FormError {
public:
    const RE::FormType expectedFormType;
    const RE::FormType receivedFormType;
    const std::string formName;

    explicit UnexpectedFormTypeError(
        RE::FormType expectedFormType,
        RE::FormType receivedFormType,
        std::string_view formName);
};

class MissingFormError : public FormError {
public:
    template <typename T>
    explicit MissingFormError(const T& formLocator)
        : FormError(
              fmt::format(FMT_STRING("Form does not exist: {}"sv), formLocator))
    {}
};
