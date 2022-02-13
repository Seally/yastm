#pragma once

#include <string>
#include <type_traits>
#include <variant>

#include <fmt/format.h>

#include <RE/T/TESDataHandler.h>
#include <RE/T/TESForm.h>

#include "FormId.hpp"

using FormLocator = std::variant<FormId, std::string>;

inline RE::TESForm* getFormForLocator(
    const FormLocator& formLocator,
    RE::TESDataHandler* const dataHandler,
    std::string_view invalidFormLocatorMessage = "Invalid form locator.")
{
    return std::visit(
        [dataHandler, invalidFormLocatorMessage](auto&& formLocator) {
            using T = std::decay_t<decltype(formLocator)>;

            if constexpr (std::is_same_v<T, FormId>) {
                return dataHandler->LookupForm(
                    formLocator.id(),
                    formLocator.pluginName());
            } else if constexpr (std::is_same_v<T, std::string>) {
                return RE::TESForm::LookupByEditorID(formLocator);
            } else {
                throw std::runtime_error(invalidFormLocatorMessage.data());
            }
        },
        formLocator);
}
