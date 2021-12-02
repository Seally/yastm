#pragma once

#include <exception>
#include <string>

#include <RE/F/FormTypes.h>

#include "FormId.hpp"

class FormError : public std::runtime_error {
public:
    explicit FormError(const std::string& message)
        : std::runtime_error(message)
    {}

    explicit FormError(const char* message)
        : std::runtime_error(message)
    {}
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
    const FormId formId;

    explicit MissingFormError(const FormId& formId);
};
