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

template <typename T>
class Form {
protected:
    std::optional<FormId> formId_;
    T* form_ = nullptr;

public:
    static constexpr auto FormType = T::FORMTYPE;

    explicit Form() {}
    virtual ~Form() {}

    void setFromToml(const toml::array& arr);
    void loadForm(RE::TESDataHandler* dataHandler);
    void clear();

    const FormId& formId() const { return formId_.value(); }
    T* form() const { return form_; }

    bool isConfigLoaded() const { return formId_.has_value(); }
    bool isFormLoaded() const { return form_ != nullptr; }
};

template <typename T>
inline void Form<T>::setFromToml(const toml::array& arr)
{
    formId_.emplace(arr);
}

template <typename T>
inline void Form<T>::loadForm(RE::TESDataHandler* const dataHandler)
{
    using namespace std::literals;

    if (!formId_.has_value()) {
        return;
    }

    const auto& formId = formId_.value();
    auto form = dataHandler->LookupForm(formId.id(), formId.pluginName());

    if (form == nullptr) {
        throw MissingFormError(formId);
    }

    const auto formType = form->GetFormType();

    if (formType == FormType) {
        form_ = form->As<T>();
    } else {
        throw UnexpectedFormTypeError(FormType, formType, form->GetName());
    }
}

template <typename T>
inline void Form<T>::clear()
{
    formId_.reset();
    form_ = nullptr;
}
