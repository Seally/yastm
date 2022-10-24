#pragma once

#include <optional>
#include <string>

#include <toml++/toml.h>

#include <RE/T/TESDataHandler.h>

#include "FormLocator.hpp"
#include "FormError.hpp"

namespace RE {
    class TESDataHandler;
} // end namespace RE

template <typename T>
class Form {
protected:
    std::optional<FormLocator> formLocator_;
    T* form_ = nullptr;

public:
    static constexpr auto FormType = T::FORMTYPE;

    explicit Form() noexcept {}
    virtual ~Form() {}

    void setFromTomlArray(const toml::array& arr);
    void setFromTomlString(std::string str);
    void loadForm(RE::TESDataHandler* dataHandler);
    void clear() noexcept
    {
        formLocator_.reset();
        form_ = nullptr;
    }

    const FormLocator& formLocator() const { return formLocator_.value(); }
    T* form() const noexcept { return form_; }

    bool isConfigLoaded() const noexcept { return formLocator_.has_value(); }
    bool isFormLoaded() const noexcept { return form_ != nullptr; }
};

template <typename T>
inline void Form<T>::setFromTomlArray(const toml::array& arr)
{
    formLocator_.emplace(FormId(arr));
}

template <typename T>
inline void Form<T>::setFromTomlString(std::string str)
{
    formLocator_.emplace(str);
}

template <typename T>
inline void Form<T>::loadForm(RE::TESDataHandler* const dataHandler)
{
    using namespace std::literals;

    if (!formLocator_.has_value()) {
        return;
    }

    const auto& formLocator = formLocator_.value();

    auto form = getFormForLocator(formLocator, dataHandler);

    if (form == nullptr) {
        std::visit(
            [](auto&& formLocator) { throw MissingFormError(formLocator); },
            formLocator);
    }

    form_ = form->As<T>();

    if (form_ == nullptr) {
        throw UnexpectedFormTypeError(
            FormType,
            form->GetFormType(),
            form->GetName());
    }
}
