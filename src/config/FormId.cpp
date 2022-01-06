#include "FormId.hpp"

#include "../utilities/stringutils.hpp"

#include "ParseError.hpp"

FormId::FormId(const toml::array& arr)
{
    auto formIdValue = arr[0].as_integer();
    auto pluginNameValue = arr[1].as_string();

    if (formIdValue == nullptr) {
        throw InvalidEntryValueTypeError(
            static_cast<std::size_t>(0),
            ValueType::Integer,
            "Form ID is missing or invalid");
    }

    if (pluginNameValue == nullptr) {
        throw InvalidEntryValueTypeError(
            static_cast<std::size_t>(1),
            ValueType::String,
            "Plugin name is missing or invalid");
    }

    id_ = static_cast<RE::FormID>(formIdValue->get());
    pluginName_ = pluginNameValue->get();
    toLowerString(pluginName_, pluginNameLower_);
}

FormId::FormId(const RE::FormID id, std::string_view pluginName)
    : id_(id)
    , pluginName_(pluginName)
{
    toLowerString(pluginName_, pluginNameLower_);
}
