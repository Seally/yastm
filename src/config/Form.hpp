#pragma once

#include <optional>
#include <string>

#include <toml++/toml.h>

#include <RE/T/TESDataHandler.h>

#include "FormId.hpp"
#include "FormError.hpp"

namespace RE {
    class TESDataHandler;
} // end namespace RE

template<typename T>
class Form {
protected:
    std::optional<FormId> _formId;
    T* _form;

public:
    static constexpr auto FormType = T::FORMTYPE;

    explicit Form() : _form{nullptr} {}
    virtual ~Form() {}

    void setFromToml(const toml::array& arr);
    void loadForm(RE::TESDataHandler* dataHandler);

    const FormId& formId() const { return _formId.value(); }
    T* form() const { return _form; }

    bool isConfigLoaded() const { return _formId.has_value(); }
    bool isFormLoaded() const { return _form != nullptr; }
};

template <typename T>
inline void Form<T>::setFromToml(const toml::array& arr)
{
    _formId.emplace(arr);
}

template <typename T>
inline void Form<T>::loadForm(RE::TESDataHandler* const dataHandler)
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

    if (formType == FormType) {
        _form = form->As<T>();
    } else {
        throw UnexpectedFormTypeError{
            FormType,
            formType,
            form->GetName()};
    }
}
